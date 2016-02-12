/*************************************************************************
 *									 *
 *	 YAP Prolog 							 *
 *									 *
 *	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
 *									 *
 * Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
 *									 *
 **************************************************************************
 *				PrologPathProkoh
 *									 *
 * File:		sysbits.c						 *
 * Last rev:	4/03/88							 *
 * mods:									 *
 * comments:	very much machine dependent routines			 *
 *									 *
 *************************************************************************/
#ifdef SCCS
static char SccsId[] = "%W% %G%";
#endif

#include "sysbits.h"

/// File Error Handler
static void
Yap_FileError(yap_error_number type, Term where, const char *format,...)
{

    if ( trueLocalPrologFlag(FILEERRORS_FLAG) ) {
        va_list ap;

        va_start (ap, format);
        /* now build the error string */
        Yap_Error(type, TermNil, format, ap);
        va_end (ap);

    }
}



static Int p_sh( USES_REGS1 );
static Int p_shell( USES_REGS1 );
static Int p_system( USES_REGS1 );
static Int p_mv( USES_REGS1 );
static Int p_dir_sp( USES_REGS1 );
static Int p_getenv( USES_REGS1 );
static Int p_putenv( USES_REGS1 );
static Term do_glob(const char *spec, bool ok_to);
#ifdef MACYAP

static int chdir(char *);
/* #define signal	skel_signal */
#endif /* MACYAP */
static char *
expandVars(const char *spec);

void exit(int);

static void
freeBuffer( const void *ptr )
{
  if (ptr == NULL ||
      ptr == LOCAL_FileNameBuf || ptr == LOCAL_FileNameBuf2)
    return;
  free((void *)ptr);
}

#ifdef _WIN32
void
Yap_WinError(char *yap_error)
{
    char msg[256];
    /* Error, we could not read time */
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 256,
                  NULL);
    Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil, "%s at %s", msg, yap_error);
}
#endif /* __WINDOWS__ */


#define is_valid_env_char(C) ( ((C) >= 'a' && (C) <= 'z') || ((C) >= 'A' && \
    (C) <= 'Z') || (C) == '_' )

#if __ANDROID__

AAssetManager * Yap_assetManager;


void *
Yap_openAssetFile( const char *path ) {
    const char * p = path+8;
    AAsset* asset = AAssetManager_open(Yap_assetManager, p, AASSET_MODE_UNKNOWN);
    return asset;
}

bool
Yap_isAsset( const char *path )
{
    if (Yap_assetManager == NULL)
        return false;
    return path[0] == '/'&&
            path[1] == 'a'&&
            path[2] == 's'&&
            path[3] == 's'&&
            path[4] == 'e'&&
            path[5] == 't'&&
            path[6] == 's'&&
            (path[7] == '/' || path[7] == '\0');
}

bool
Yap_AccessAsset( const char *name, int mode )
{
    AAssetManager* mgr = Yap_assetManager;
    const char *bufp=name+7;

    if (bufp[0] == '/')
        bufp++;
    if ((mode & W_OK) == W_OK) {
        return false;
    }
    // directory works if file exists
    AAssetDir *assetDir = AAssetManager_openDir(mgr, bufp);
    if (assetDir) {
        AAssetDir_close(assetDir);
        return true;
    }
    return false;
}

bool
Yap_AssetIsFile( const char *name )
{
    AAssetManager* mgr = Yap_assetManager;
    const char *bufp=name+7;
    if (bufp[0] == '/')
        bufp++;
    // check if file is a directory.
    AAsset *asset = AAssetManager_open(mgr, bufp, AASSET_MODE_UNKNOWN);
    if (!asset)
        return false;
    AAsset_close(asset);
    return true;
}

bool
Yap_AssetIsDir( const char *name )
{
    AAssetManager* mgr = Yap_assetManager;
    const char *bufp=name+7;
    if (bufp[0] == '/')
        bufp++;
    // check if file is a directory.
    AAssetDir *assetDir = AAssetManager_openDir(mgr, bufp);
    if (!assetDir) {
        return false;
    }
    AAssetDir_close(assetDir);
    AAsset *asset = AAssetManager_open(mgr, bufp, AASSET_MODE_UNKNOWN);
    if (!asset)
        return true;
    AAsset_close(asset);
    return false;
}

int64_t
Yap_AssetSize( const char *name )
{
    AAssetManager* mgr = Yap_assetManager;
    const char *bufp=name+7;
    if (bufp[0] == '/')
        bufp++;
    AAsset *asset = AAssetManager_open(mgr, bufp, AASSET_MODE_UNKNOWN);
    if (!asset)
        return -1;
    off64_t len = AAsset_getLength64(asset);
    AAsset_close(asset);
    return len;
}
#endif



/// is_directory: verifies whether an expanded file name
/// points at a readable directory
static bool
is_directory(const char *FileName)
{
#ifdef __ANDROID__
    if (Yap_isAsset(FileName)) {
        return Yap_AssetIsDir(FileName);
    }

#endif

#ifdef __WINDOWS__
    DWORD dwAtts = GetFileAttributes( FileName );
    if (dwAtts == INVALID_FILE_ATTRIBUTES)
        return false;
    return (dwAtts & FILE_ATTRIBUTE_DIRECTORY);
#elif HAVE_LSTAT
    struct stat buf;

    if (lstat(FileName, &buf) == -1) {
        /* return an error number */
        return false;
    }
    return S_ISDIR(buf.st_mode);
#else
    Yap_Error(SYSTEM_ERROR_INTERNAL, TermNil,
              "stat not available in this configuration");
    return false;
#endif
}

/// has_access just calls access
/// it uses F_OK, R_OK and friend
static bool
has_access(const char *FileName, int mode)
{
#ifdef __ANDROID__
    if (Yap_isAsset(FileName)) {
        return Yap_AccessAsset(FileName, mode);
    }
#endif
#if HAVE_ACCESS
    if (access( FileName, mode ) == 0)
        return true;
    if (errno == EINVAL) {
        Yap_Error(SYSTEM_ERROR_INTERNAL, TermNil,
                  "bad flags to access");
    }
    return false;
#else
    Yap_Error(SYSTEM_ERROR_INTERNAL, TermNil,
              "access not available in this configuration");
    return false;
#endif
}

static bool
exists( const char *f)
{
    return has_access( f, F_OK );
}

static int
dir_separator (int ch)
{
#ifdef MAC
    return (ch == ':');
#elif ATARI || _MSC_VER
    return (ch == '\\');
#elif defined(__MINGW32__) || defined(__CYGWIN__)
    return (ch == '\\' || ch == '/');
#else
    return (ch == '/');
#endif
}

int
Yap_dir_separator (int ch)
{
    return dir_separator (ch);
}

#if __WINDOWS__
#include <psapi.h>

char *libdir = NULL;
#endif


bool
Yap_IsAbsolutePath(const char *p0)
{
    // verify first if expansion is needed: ~/ or $HOME/
    char *p = expandVars( p0 );
    bool nrc;
#if _WIN32 || __MINGW32__
    nrc = !PathIsRelative(p);
#else
    nrc =  ( p[0] == '/' );
#endif
    return nrc;
}

#define isValidEnvChar(C) ( ((C) >= 'a' && (C) <= 'z') || ((C) >= 'A' && \
    (C) <= 'Z') || (C) == '_' )

// this is necessary because
// support for ~expansion at the beginning
// systems like Android do not do this.
static char *
PlExpandVars (const char *source)
{
    const char *src = source;
    char *result = LOCAL_FileNameBuf;

    if (strlen(source) >= YAP_FILENAME_MAX) {
        Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil, "%s in true_file-name is larger than the buffer size (%d bytes)", source, strlen(source));
    }
    /* step 1: eating home information */
    if (source[0] == '~') {
        if (dir_separator(source[1]) || source[1] == '\0')
        {
            char *s;
            src++;
#if defined(_WIN32)
            s = getenv("HOMEDRIVE");
            if (s != NULL)
                strncpy (result, getenv ("HOMEDRIVE"), YAP_FILENAME_MAX);
            //s = getenv("HOMEPATH");
#else
            s = getenv ("HOME");
#endif
            if (s != NULL)
                strncpy (result, s, YAP_FILENAME_MAX);
            strcat(result,src);
            return result;
        } else {
#if HAVE_GETPWNAM
            struct passwd *user_passwd;
            char *res = result;

            src++;
            while (!dir_separator((*res = *src)) && *res != '\0')
                res++, src++;
            res[0] = '\0';
            if ((user_passwd = getpwnam (result)) == NULL) {
                Yap_FileError(SYSTEM_ERROR_OPERATING_SYSTEM, MkAtomTerm(Yap_LookupAtom(source)),"User %s does not exist in %s", result, source);
                return NULL;
            }
            strncpy (result, user_passwd->pw_dir, YAP_FILENAME_MAX);
            strcat(result, src);
#else
            Yap_FileError(SYSTEM_ERROR_OPERATING_SYSTEM, MkAtomTerm(Yap_LookupAtom(source)),"User %s cannot be found in %s, missing getpwnam", result, source);
            return NULL;
#endif
        }
        return result;
    }
    // do VARIABLE expansion
    else if (source[0] == '$') {
        /* follow SICStus expansion rules */
        char v[YAP_FILENAME_MAX+1];
        int ch;
        char *s, *res;
        src = source+1;
        if (src[0] == '{') {
            res = v;
            src++;
            while ((*res = (ch = *src++)) && isValidEnvChar (ch) && ch != '}') {
                res++;
            }
            if (ch == '}') {
                // {...}
                // done
                res[0] = '\0';
            }
        } else {
            res = v;
            while ((*res = (ch = *src++)) && isValidEnvChar (ch) && ch != '}') {
                res++;
            }
            src--;
            res[0] = '\0';
        }
        if ((s = (char *) getenv (v))) {
            strcpy (result, s);
            strcat (result, src);
        } else
            strcpy( result, src);
    }
    else {
        strncpy (result, source, YAP_FILENAME_MAX);
    }
    return result;
}

#if _WIN32 || defined(__MINGW32__)
// straightforward conversion from Unix style to WIN style
// check cygwin path.cc for possible improvements
static char *
unix2win( const char *source, char *target, int max)
{
    char *s = target;
    const char *s0 = source;
    char *s1;
    int ch;

    if (s == NULL)
        s = malloc(YAP_FILENAME_MAX+1);
    s1 = s;
    // win32 syntax
    // handle drive notation, eg //a/
    if (s0[0] == '\0') {
        s[0] = '.';
        s[1] = '\0';
        return s;
    }
    if (s0[0] == '/' &&  s0[1] == '/' && isalpha(s0[2]) && s0[3] == '/')
    {
        s1[0] = s0[2];
        s1[1] = ':';
        s1[2] = '\\';
        s0+=4;
        s1+=3;
    }
    while ((ch = *s1++ = *s0++)) {
        if (ch == '$') {
            s1[-1] = '%';
            ch = *s0;
            // handle $(....)
            if (ch == '{') {
                s0++;
                while ((ch = *s0++) != '}') {
                    *s1++ = ch;
                    if (ch == '\0') return FALSE;
                }
                *s1++ = '%';
            } else {
                while (((ch = *s1++ = *s0++) >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch == '-') || (ch >= '0' && ch <= '9') || (ch == '_'));
                s1[-1] = '%';
                *s1++ = ch;
                if (ch == '\0') { s1--; s0--; }
            }
        } else if (ch == '/')
            s1[-1] = '\\';
    }
    return s;
}
#endif


static char *
OsPath(const char *p, char *buf)
{
    return (char *)p;
}

static char *
PrologPath(const char *Y, char *X) {
    return (char *)Y  ;
}


#if _WIN32
#define HAVE_BASENAME 1
#define HAVE_REALPATH 1
#endif

static bool ChDir(const char *path) {
    bool rc = false;
    const char *qpath = Yap_AbsoluteFile(path, true);

#ifdef __ANDROID__
    if (GLOBAL_AssetsWD) {
        freeBuffer( GLOBAL_AssetsWD );
        GLOBAL_AssetsWD = NULL;
    }
    if (Yap_isAsset(qpath) ) {
        AAssetManager* mgr = Yap_assetManager;

        const char *ptr = qpath+8;
        AAssetDir* d;
        if (ptr[0] == '/')
            ptr++;
        d = AAssetManager_openDir(mgr, ptr);
        if (d) {
            GLOBAL_AssetsWD = malloc( strlen(qpath) + 1 );
            strcpy( GLOBAL_AssetsWD, qpath );
            AAssetDir_close( d );
            return true;
        }
        return false;
    } else {
        GLOBAL_AssetsWD = NULL;
    }
#endif
#if _WIN32 || defined(__MINGW32__)

    if ((rc = (SetCurrentDirectory(qpath) != 0)) == 0)
    {
        Yap_WinError("SetCurrentDirectory failed" );
    }
#else
    rc = (chdir(qpath) == 0);
#endif
    free((char *)qpath);
    return rc;
}
#if _WIN32 || defined(__MINGW32__)
char *
BaseName(const char *X) {
    char *qpath = unix2win(X, NULL, YAP_FILENAME_MAX);
    char base[YAP_FILENAME_MAX], ext[YAP_FILENAME_MAX];
    _splitpath(qpath, NULL, NULL, base, ext);
    strcpy(qpath, base);
    strcat(qpath, ext);
    return qpath;
}


const char *
DirName(const char *X) {
    char dir[YAP_FILENAME_MAX];
    char drive[YAP_FILENAME_MAX];
    char *o = unix2win(X, NULL, YAP_FILENAME_MAX);
    int err;
    if (!o)
        return NULL;
    if (( err = _splitpath_s(o, drive, YAP_FILENAME_MAX-1, dir, YAP_FILENAME_MAX-1,NULL, 0, NULL, 0) ) != 0) {
        Yap_FileError(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil, "could not perform _splitpath %s: %s", X, strerror(errno));
        return NULL;

    }
    strncpy(o, drive, YAP_FILENAME_MAX-1);
    strncat(o, dir, YAP_FILENAME_MAX-1);
    return o;
}
#endif

static const char *myrealpath( const char *path)
{
#if _WIN32 || defined(__MINGW32__)
    DWORD  retval=0;

    // notice that the file does not need to exist
    retval = GetFullPathName(path,
                             YAP_FILENAME_MAX,
                             out,
                             NULL);

    if (retval == 0)
    {
        Yap_WinError("Generating a full path name for a file" );
        return NULL;
    }
    return out;
#elif HAVE_REALPATH
    {
        char * rc = realpath(path,NULL);

        if (rc) {
            return rc;
        }
        // rc = NULL;
        if (errno == ENOENT|| errno == EACCES) {
	  char base[YAP_FILENAME_MAX];
	  strncpy(base, path, YAP_FILENAME_MAX-1);
	  rc = realpath( dirname( (char *)path ), NULL);

	  if (rc) {
	    const char *b = basename(base);
	    size_t e = strlen(rc);
	    size_t bs = strlen( b );

	    rc = realloc( rc , e+bs+2);
#if _WIN32
	    if (rc[e-1] != '\\' && rc[e-1] != '/' ) {
	      rc[e]='\\';
	      rc[e+1]='\0';
	    }
#else
	    if (rc[e-1] != '/' ) {
	      rc[e]='/';
	      rc[e+1]='\0';
	    }
#endif
	    strcat(rc, b);
	    return rc;
	  }
        }
    }
#endif
    char *out = malloc(strlen(path)+1);
    strcpy( out, path);
    return out;
}

static char *
expandVars(const char *spec)
{
#if _WIN32 || defined(__MINGW32__)
    char u[YAP_FILENAME_MAX+1];

    // first pass, remove Unix style stuff
    if ((ou=unix2win(spec, YAP_FILENAME_MAX)) == NULL)
        return NULL;
    spec = u;
#endif
    bool  ok_to = true;
    if (spec == NULL) {
        return NULL;
    }
    if ( ok_to )
    {
        Term t = do_glob( spec, true );
        if (IsPairTerm(t))
	  return RepAtom(AtomOfTerm(HeadOfTerm(t)))->StrOfAE;
        return NULL;
    } else {
      return PlExpandVars( spec );
    }
    return (char *)spec;
}

/**
 * generate absolute path, if ok first expand SICStus Prolog style
 *
 * @param[in]  spec the file path, including `~` and `$`.
 * @param[in]  ok where to process `~` and `$`.
 *
 * @return tmp, or NULL, in malloced memory
 */
const char *
Yap_AbsoluteFile(const char *spec, bool ok)
{
    const char*p;
    const char*rc;
    rc = expandVars(spec);
    if (!rc)
        return spec;
    if ((p = myrealpath(rc) ) ) {
        return p;
    } else {
        return NULL;
    }
}

/**
 * generate absolute path and stores path in an user given buffer. If 
 * NULL, uses a temporary buffer that must be quickly released.
 *
 * if ok first expand variable names and do globbing
 *
 * @param[in]  spec the file path, including `~` and `$`.
 * @param[in]  ok where to process `~` and `$`.
 *
 * @return tmp, or NULL, in malloced memory
 */
const char *
Yap_AbsoluteFileInBuffer(const char *spec, char *out, size_t sz, bool ok)
{
    const char*p;
    const char*rc;
    if (ok) {
      rc = expandVars(spec);
    if (!rc)
        return spec;
    } else {
      rc = spec;
    }
    
    if ((p = myrealpath(rc) ) ) {
      if (!out) {
	out = LOCAL_FileNameBuf;
	sz = YAP_FILENAME_MAX-1;
      }
      if (p != out ) {
	strncpy(out, p, sz);
	freeBuffer(p);
        return out;
      } else {
        return NULL;
      }
    }
    return NULL;
}

static Term
/* Expand the string for the program to run.  */
do_glob(const char *spec, bool glob_vs_wordexp)
{
#if _WIN32 || defined(__MINGW32__)
    {
        char u[YAP_FILENAME_MAX+1];
        WIN32_FIND_DATA find;
        HANDLE hFind;
        CELL *dest;

        // first pass, remove Unix style stuff
        if (unix2win(spec, u, YAP_FILENAME_MAX) == NULL)
            return TermNil;
        spec = (const char *)u;

        if (!use_system_expansion) {
            return MkPairTerm(MkAtomTerm(Yap_LookupAtom(spec)), TermNil);
        }
        hFind = FindFirstFile(spec, &find);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            return TermNil;
        }
        else
        {
            tf  = AbsPair(HR);
            HR[0] = MkAtomTerm(Yap_LookupAtom(find.cFileName));
            HR[1] = TermNil;
            dest = HR+1;
            HR += 2;
            while (FindNextFile(hFind, &find)) {
                *dest  = AbsPair(HR);
                HR[0] = MkAtomTerm(Yap_LookupAtom(find.cFileName));
                HR[1] = TermNil;
                dest = HR+1;
                HR += 2;
            }
            FindClose(hFind);
        }
        return tf;
    }
#elif HAVE_WORDEXP || HAVE_GLOB
    /* Expand the string for the program to run.  */
    size_t pathcount;
#if HAVE_GLOB
    glob_t gresult;
#endif
#if HAVE_WORDEXP
    wordexp_t wresult;
#endif
#if HAVE_GLOB || HAVE_WORDEXP
    char **ss = NULL;
    int flags = 0,  j;
#endif
    if ( glob_vs_wordexp ) {
#if HAVE_GLOB
#ifdef GLOB_NOCHECK
        flags = GLOB_NOCHECK;
#else
        flags = 0;
#endif
#ifdef GLOB_BRACE
        flags |= GLOB_BRACE|GLOB_TILDE;
#endif
        switch (glob (spec, flags, NULL, &gresult))
        {
        case 0:                     /* Successful.  */
            ss = gresult.gl_pathv;
            pathcount = gresult.gl_pathc;
            if (pathcount) {
                break;
            }
        case GLOB_NOMATCH:
            globfree(&gresult);
        {
            return TermNil;
        }
        case GLOB_ABORTED:
            PlIOError(SYSTEM_ERROR_OPERATING_SYSTEM, ARG1, "glob aborted: %sn", strerror(errno));
            globfree (&gresult);
            return TermNil;
        case GLOB_NOSPACE:
            Yap_Error(RESOURCE_ERROR_HEAP, ARG1, "glob ran out of space: %sn", strerror(errno));
            globfree (&gresult);
            return TermNil;
            /* If the error was WRDE_NOSPACE,
             then perhaps part of the result was allocated.  */
        default:                    /* Some other error.  */
            return TermNil;
        }
#endif
    } else {
#if HAVE_WORDEXP
        int rc;
        memset( &wresult,0,sizeof(wresult) );
        switch ((rc = wordexp (spec, &wresult, flags)))
        {
        case 0:                     /* Successful.  */
            ss = wresult.we_wordv;
            pathcount = wresult.we_wordc;
            if (pathcount) {
                break;
            } else {
                Term t;
                t = MkAtomTerm( Yap_LookupAtom( expandVars(spec) ) );
                wordfree (&wresult);
                return MkPairTerm( t, TermNil );
            }
        case WRDE_NOSPACE:
            /* If the error was WRDE_NOSPACE,
             then perhaps part of the result was allocated.  */
            Yap_Error(RESOURCE_ERROR_HEAP, ARG1, "wordexp ran out of space: %s", strerror(errno));
            wordfree (&wresult);
            return TermNil;
        default:                    /* Some other error.  */
            PlIOError(SYSTEM_ERROR_OPERATING_SYSTEM, ARG1, "wordexp failed: %s", strerror(errno));
            wordfree (&wresult);
            return TermNil;
        }
#endif
    }
    const char * tmp;
    Term tf = TermNil;
    for (j = 0; j < pathcount; j++) {
        const char *s = ss[pathcount-(j+1)];
#if HAVE_REALPATH
        tmp = myrealpath(s);
#else
        tmp = s;
#endif
        //if (!exists(s))
        //  continue;
        Atom a = Yap_LookupAtom(tmp);
        tf = MkPairTerm(MkAtomTerm( a ),tf);
    }
#if HAVE_GLOB
    if ( glob_vs_wordexp)
        globfree( &gresult );
#endif
#if HAVE_WORDEXP
    if ( !glob_vs_wordexp)
        wordfree( &wresult );
#endif
    if (tmp)
        freeBuffer( (void *)tmp );
    return tf;
#else
    // just use basic
    return MkPairTerm(MkAtomTerm(Yap_LookupAtom(spec)), TermNil);
#endif
}

/**
 * @pred prolog_expanded_file_system_path( +PrologPath, +ExpandVars, -OSPath )
 *
 * Apply basic transformations to paths, and conidtionally apply
 * traditional SICStus-style variable expansion.
 *
 * @param PrologPath the source, may be atom or string
 * @param ExpandVars expand initial occurrence of ~ or $
 * @param Prefix add this path before _PrologPath_
 * @param OSPath pathname.
 *
 * @return
 */
static Int
prolog_realpath( USES_REGS1 )
{
    Term t1 = Deref(ARG1);
    const char *cmd;

    if (IsAtomTerm(t1)) {
        cmd = RepAtom(AtomOfTerm(t1))->StrOfAE;
    } else if (IsStringTerm(t1)) {
        cmd = StringOfTerm(t1);
    } else {
        return false;
    }
    const char *rc = myrealpath( cmd );
    if (!rc) {
        PlIOError( SYSTEM_ERROR_OPERATING_SYSTEM, ARG1, strerror(errno));
        return false;
    }
    bool r = Yap_unify(MkAtomTerm(Yap_LookupAtom( rc )), ARG2);
    freeBuffer( (char *) rc );
    return r;

}

#define EXPAND_FILENAME_DEFS()                                                      \
    PAR("parameter_expansion", isatom, EXPAND_FILENAME_PARAMETER_EXPANSION),                                      \
    PAR("commands", booleanFlag, EXPAND_FILENAME_COMMANDS),          \
    PAR(NULL, ok, EXPAND_FILENAME_END)

#define PAR(x, y, z) z

typedef enum expand_filename_enum_choices {
    EXPAND_FILENAME_DEFS()
} expand_filename_enum_choices_t;


#undef PAR

#define PAR(x, y, z)                                                           \
{ x, y, z }

static const param_t expand_filename_defs[] = {EXPAND_FILENAME_DEFS()};
#undef PAR

static Term
do_expand_file_name(Term t1, Term opts USES_REGS)
{
    xarg *args;
    expand_filename_enum_choices_t i;
    bool use_system_expansion = true;
    char *tmpe = NULL;
    const char *spec;
    Term tf;

    if (IsVarTerm(t1)) {
        Yap_Error(INSTANTIATION_ERROR, t1, NULL);
        return TermNil;
    } else if (IsAtomTerm(t1)) {
        spec = AtomTermName( t1 );
    } else if (IsStringTerm(t1)) {
        spec = StringOfTerm( t1 );
    } else {
        Yap_Error(TYPE_ERROR_ATOM, t1, NULL);
        return TermNil;
    }
    args = Yap_ArgListToVector(opts, expand_filename_defs, EXPAND_FILENAME_END);
    if (args == NULL) {
        return TermNil;
    }
    tmpe = malloc(YAP_FILENAME_MAX+1);

    for (i = 0; i < EXPAND_FILENAME_END; i++) {
        if (args[i].used) {
            Term t = args[i].tvalue;
            switch (i) {
            case EXPAND_FILENAME_PARAMETER_EXPANSION:
                if (t == TermProlog) {
                    char *s = expandVars( spec);
                    if (s == NULL) {
                        return TermNil;
                    }
                    strcpy(tmpe, s);
                } else if (t == TermTrue) {
                    use_system_expansion = true;
                } else if (t == TermFalse) {
                    use_system_expansion = false;
                }
                break;
            case EXPAND_FILENAME_COMMANDS:
                if (!use_system_expansion) {
                    use_system_expansion = true;
#if 0 //def WRDE_NOCMD
                    if (t == TermFalse) {
                        flags = WRDE_NOCMD;
                    }
#endif
                }
            case EXPAND_FILENAME_END:
                break;
            }
        }
    }


    if (!use_system_expansion) {
        return MkPairTerm(MkAtomTerm(Yap_LookupAtom(spec)), TermNil);
    }
    tf = do_glob(spec, true);
    return tf;
}

static Int
expand_file_name( USES_REGS1)
{
    Term tf = do_expand_file_name( Deref(ARG1), TermNil PASS_REGS);
    return
            Yap_unify( tf, ARG2);
}

static Int
expand_file_name3( USES_REGS1)
{
    Term tf = do_expand_file_name( Deref(ARG1), Deref(ARG2)  PASS_REGS);
    return
            Yap_unify( tf, ARG3 );
}

/*
       static char *canoniseFileName( char *path) {
       #if HAVE_REALPATH && HAVE_BASENAME
       #if _WIN32 || defined(__MINGW32__)
         char *o = malloc(YAP_FILENAME_MAX+1);
         if (!o)
           return NULL;
         // first pass, remove Unix style stuff
         if (unix2win(path, o, YAP_FILENAME_MAX) == NULL)
           return NULL;
         path = o;
       #endif
         char *rc;
         if (tmp == NULL) return NULL;
         rc = myrealpath(path);
       #if _WIN32 || defined(__MINGW32__)
         freeBuffer(o);
#endif
  return rc;
#endif
}
*/


static Int
absolute_file_system_path( USES_REGS1 )
{
    Term t = Deref(ARG1);
    const char *fp;
    bool rc;
    char s[MAXPATHLEN+1];

    if (IsVarTerm(t)) {
        Yap_Error(INSTANTIATION_ERROR, t, "absolute_file_system_path");
        return false;
    } else if (!IsAtomTerm(t)) {
        Yap_Error(TYPE_ERROR_ATOM, t, "absolute_file_system_path");
        return false;
    }
    if (!(fp = Yap_AbsoluteFile( RepAtom(AtomOfTerm(t))->StrOfAE, true)))
        return false;
    rc = Yap_unify(MkAtomTerm(Yap_LookupAtom(fp)), ARG2);
    if (fp != s)
        freeBuffer( (void *)fp );
    return rc;
}

static Int
prolog_to_os_filename( USES_REGS1 )
{
    Term t = Deref(ARG1), t2 = Deref(ARG2);
    char *fp;
    char out[MAXPATHLEN+1];

    if (IsVarTerm(t)) {

        if (IsVarTerm(t2)) {
            Yap_Error(INSTANTIATION_ERROR, t, "prolog_to_os_filename");
            return false;
        } else if ( IsAtomTerm(t2) ) {
            if (!(fp =   PrologPath( RepAtom(AtomOfTerm(t2))->StrOfAE, out)))
                return false;
            return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom( fp )));
        } else {
            Yap_Error(TYPE_ERROR_ATOM, t2, "prolog_to_os_filename");
            return false;
        }
    } else if (!IsAtomTerm(t)) {
        Yap_Error(TYPE_ERROR_ATOM, t, "prolog_to_os_filename");
        return false;
    }

    if (!(fp =   OsPath( RepAtom(AtomOfTerm(t))->StrOfAE, out)))
        return false;
    return Yap_unify(MkAtomTerm(Yap_LookupAtom(fp)), ARG2);
}

Atom Yap_TemporaryFile( const char *prefix, int *fd) {
#if HAVE_MKSTEMP
    char *tmp = malloc(PATH_MAX);
    int n;
    int f;
    if (tmp == NULL) return NIL;
    strncpy(tmp, prefix, PATH_MAX-1);
    n = strlen( tmp );
    if (n >= 6 &&
            tmp[n-1] == 'X' &&
            tmp[n-2] == 'X' &&
            tmp[n-3] == 'X' &&
            tmp[n-4] == 'X' &&
            tmp[n-5] == 'X' &&
            tmp[n-6] == 'X')
        f = mkstemp(tmp);
    else {
        strncat(tmp, "XXXXXX", PATH_MAX-1);
        f = mkstemp(tmp);
    }
    if (fd) *fd = f;
    return  Yap_LookupAtom(tmp);
#else
    return AtomNil;
#endif
}

/** @pred make_directory(+ _Dir_)

Create a directory  _Dir_. The name of the directory must be an atom.

*/
static Int
make_directory( USES_REGS1 )
{
    const char *fd = AtomName(AtomOfTerm(ARG1));
#if defined(__MINGW32__) || _MSC_VER
    if (_mkdir(fd) == -1) {
#else
    if (mkdir(fd, 0777) == -1) {
#endif
        /* return an error number */
        return false; // errno?
    }
    return true;
}


static Int
p_rmdir( USES_REGS1 )
{
    const char *fd = AtomName(AtomOfTerm(ARG1));
#if defined(__MINGW32__) || _MSC_VER
    if (_rmdir(fd) == -1) {
#else
    if (rmdir(fd) == -1) {
#endif
        /* return an error number */
        return(Yap_unify(ARG2, MkIntTerm(errno)));
    }
    return true;
}

static bool
initSysPath(Term tlib, Term tcommons, bool dir_done, bool commons_done) {
    CACHE_REGS
            int len;

#if __WINDOWS__
    {
        char *dir;
        if ((dir = Yap_RegistryGetString("library")) &&
                is_directory(dir)) {
            if (! Yap_unify( tlib,
                             MkAtomTerm(Yap_LookupAtom(dir))) )
                return FALSE;
        }
        dir_done = true;
        if ((dir = Yap_RegistryGetString("prolog_commons")) &&
                is_directory(dir)) {
            if (! Yap_unify( tcommons,
                             MkAtomTerm(Yap_LookupAtom(dir))) )
                return FALSE;
        }
        commons_done = true;
    }
    if (dir_done && commons_done)
        return TRUE;
#endif
    strncpy(LOCAL_FileNameBuf, YAP_SHAREDIR, YAP_FILENAME_MAX);
    strncat(LOCAL_FileNameBuf,"/", YAP_FILENAME_MAX);
    len = strlen(LOCAL_FileNameBuf);
    if (!dir_done) {
        strncat(LOCAL_FileNameBuf, "Yap", YAP_FILENAME_MAX);
        if (is_directory(LOCAL_FileNameBuf))
        {
            if (! Yap_unify( tlib,
                             MkAtomTerm(Yap_LookupAtom(LOCAL_FileNameBuf))) )
                return FALSE;
            dir_done = true;
        }
    }
    if (!commons_done) {
        LOCAL_FileNameBuf[len] = '\0';
        strncat(LOCAL_FileNameBuf, "PrologCommons", YAP_FILENAME_MAX);
        if (is_directory(LOCAL_FileNameBuf)) {
            if (! Yap_unify( tcommons,
                             MkAtomTerm(Yap_LookupAtom(LOCAL_FileNameBuf))) )
                return FALSE;
        }
        commons_done = true;
    }
    if (dir_done && commons_done)
        return TRUE;

#if __WINDOWS__
    {
        size_t buflen;
        char *pt;
        /* couldn't find it where it was supposed to be,
       let's try using the executable */
        if (!GetModuleFileName( NULL, LOCAL_FileNameBuf, YAP_FILENAME_MAX)) {
            Yap_WinError( "could not find executable name" );
            /* do nothing */
            return FALSE;
        }
        buflen = strlen(LOCAL_FileNameBuf);
        pt = LOCAL_FileNameBuf+buflen;
        while (*--pt != '\\') {
            /* skip executable */
            if (pt == LOCAL_FileNameBuf) {
                Yap_FileError(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil, "could not find executable name");
                /* do nothing */
                return FALSE;
            }
        }
        while (*--pt != '\\') {
            /* skip parent directory "bin\\" */
            if (pt == LOCAL_FileNameBuf) {
                Yap_FileError(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil, "could not find executable name");
                /* do nothing */
                return FALSE;
            }
        }
        /* now, this is a possible location for the ROOT_DIR, let's look for a share directory here */
        pt[1] = '\0';
        /* grosse */
        strncat(LOCAL_FileNameBuf,"lib\\Yap",YAP_FILENAME_MAX);
        libdir = Yap_AllocCodeSpace(strlen(LOCAL_FileNameBuf)+1);
        strncpy(libdir, LOCAL_FileNameBuf, strlen(LOCAL_FileNameBuf)+1);
        pt[1] = '\0';
        strncat(LOCAL_FileNameBuf,"share",YAP_FILENAME_MAX);
    }
    strncat(LOCAL_FileNameBuf,"\\", YAP_FILENAME_MAX);
    len = strlen(LOCAL_FileNameBuf);
    strncat(LOCAL_FileNameBuf, "Yap", YAP_FILENAME_MAX);
    if (!dir_done && is_directory(LOCAL_FileNameBuf)) {
        if (! Yap_unify( tlib,
                         MkAtomTerm(Yap_LookupAtom(LOCAL_FileNameBuf))) )
            return FALSE;
    }
    dir_done = true;
    LOCAL_FileNameBuf[len] = '\0';
    strncat(LOCAL_FileNameBuf, "PrologCommons", YAP_FILENAME_MAX);
    if (!commons_done && is_directory(LOCAL_FileNameBuf)) {
        if (! Yap_unify( tcommons,
                         MkAtomTerm(Yap_LookupAtom(LOCAL_FileNameBuf))) )
            return FALSE;
    }
    commons_done = true;
#endif
    return dir_done && commons_done;
}


static Int
libraries_directories( USES_REGS1 )
{
    return initSysPath( ARG1, ARG2 , false, false );
}


static Int
system_library( USES_REGS1 )
{
    return initSysPath( ARG1, MkVarTerm(), false, true );
}

static Int
commons_library( USES_REGS1 )
{
    return initSysPath( MkVarTerm(), ARG1, true, false );
}

static Int
p_dir_sp ( USES_REGS1 )
{
#if ATARI || _MSC_VER || defined(__MINGW32__)
    Term t = MkIntTerm('\\');
    Term t2 = MkIntTerm('/');
#else
    Term t = MkIntTerm('/');
    Term t2 = MkIntTerm('/');
#endif

    return Yap_unify_constant(ARG1,t) || Yap_unify_constant(ARG1,t2) ;
}


void
Yap_InitPageSize(void)
{
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    Yap_page_size = si.dwPageSize;
#elif HAVE_UNISTD_H
#if defined(__FreeBSD__) || defined(__DragonFly__)
    Yap_page_size = getpagesize();
#elif defined(_AIX)
    Yap_page_size = sysconf(_SC_PAGE_SIZE);
#elif !defined(_SC_PAGESIZE)
    Yap_page_size = getpagesize();
#else
    Yap_page_size = sysconf(_SC_PAGESIZE);
#endif
#else
    bla bla
        #endif
}



/* TrueFileName -> Finds the true name of a file */

 bool Yap_trueFileName(const char *isource, const char *idef, const char *root,
                      char *result, bool access, file_type_t ftype,
                      bool expand_root, bool in_lib);

#ifdef __MINGW32__
#include <ctype.h>
#endif

static int
volume_header(char *file)
{
#if _MSC_VER || defined(__MINGW32__)
    char *ch = file;
    int c;

    while ((c = ch[0]) != '\0') {
        if (isalnum(c)) ch++;
        else return(c == ':');
    }
#endif
    return(FALSE);
}

int
Yap_volume_header(char *file)
{
    return volume_header(file);
}

const char * Yap_getcwd(const char *cwd, size_t cwdlen)
{
#if _WIN32 || defined(__MINGW32__)
    if (GetCurrentDirectory(cwdlen, (char *)cwd) == 0)
    {
        Yap_WinError("GetCurrentDirectory failed" );
        return NULL;
    }
    return (char *)cwd;
#elif __ANDROID__
    if (GLOBAL_AssetsWD) {
        return strncpy( (char *)cwd, (const char *)GLOBAL_AssetsWD, cwdlen);
    }

#endif
    return getcwd((char *)cwd, cwdlen);
}

static Int
working_directory(USES_REGS1)
{
    char dir[YAP_FILENAME_MAX+1];
    Term t1 = Deref(ARG1), t2;
    if ( !IsVarTerm( t1 ) && !IsAtomTerm(t1) ) {
        Yap_Error(TYPE_ERROR_ATOM, t1, "working_directory");
    }
    if (!Yap_unify( t1, MkAtomTerm(Yap_LookupAtom(Yap_getcwd(dir,YAP_FILENAME_MAX )))) )
        return false;
    t2 = Deref(ARG2);
    if ( IsVarTerm( t2 ) ) {
        Yap_Error(INSTANTIATION_ERROR, t2, "working_directory");
    }
    if ( !IsAtomTerm(t2) ) {
        Yap_Error(TYPE_ERROR_ATOM, t2, "working_directory");
    }
    ChDir(RepAtom(AtomOfTerm(t2))->StrOfAE);
    return true;
}

static const char *
expandWithPrefix(const char *source, const char *root)
{
    char *work;

    work = expandVars( source );
    // expand names first
    if (root && !Yap_IsAbsolutePath( source ) ) {
        char *s = expandVars( source);
        if (!s)
            return source;
        char *r0 = expandVars( root);
        size_t sl = strlen(s);
        size_t rl = strlen(r0);
        char *r = malloc( sl+rl+2);
        strncat( r,  r0, sl+rl+2 );
        strncat( r, "/", sl+rl+2 );
        strncat( r, s , sl+rl+2);
        return r;
    }
    strncpy( LOCAL_FileNameBuf, work, MAXPATHLEN-1);
    return LOCAL_FileNameBuf;
}

/** Yap_trueFileName: tries to generate the true name of  file
   aaaaaaaaaaaaaaaaaaaaa*
   *
   * @param isource the proper file
   * @param idef the default name fo rthe file, ie, startup.yss
   * @param root the prefix
   * @param result the output
   * @param access verify whether the file has access permission
   * @param ftype saved state, object, saved file, prolog file
   * @param expand_root expand $ ~, etc
   * @param in_lib library file
   *
   * @return
   */
bool
Yap_trueFileName (const char *isource, const char * idef,  const char *iroot, char *result, bool access, file_type_t ftype, bool expand_root, bool in_lib)
{

    char save_buffer[YAP_FILENAME_MAX+1];
    const char *root, *source = isource;
    int rc = FAIL_RESTORE;
    int try       = 0;

    while ( rc == FAIL_RESTORE) {
        bool done = false;
        // { CACHE_REGS __android_log_print(ANDROID_LOG_ERROR,  __FUNCTION__, "try=%d %s %s", try, isource, iroot) ; }
        switch (try++) {
        case 0:  // path or file name is given;
            root = iroot;
            if (iroot || isource) {
                source = ( isource ? isource : idef ) ;
            } else {
                done = true;
            }
            break;
        case 1: // library directory is given in command line
            if ( in_lib && ftype == YAP_SAVED_STATE) {
                root = iroot;
                source = ( isource ? isource : idef ) ;
            } else
                done = true;
            break;
        case 2: // use environment variable YAPLIBDIR
#if HAVE_GETENV
            if ( in_lib) {
                if (ftype == YAP_SAVED_STATE || ftype == YAP_OBJ) {
                    root = getenv("YAPLIBDIR");
                } else {
                    root = getenv("YAPSHAREDIR");
                }
                source = ( isource ? isource : idef ) ;
            } else
                done = true;
            break;
#else
            done = true;
#endif
            break;
        case 3: // use compilation variable YAPLIBDIR
            if ( in_lib) {
                source = ( isource ? isource : idef ) ;
                if (ftype == YAP_PL || ftype == YAP_QLY) {
                    root = YAP_SHAREDIR;
                } else {
                    root = YAP_LIBDIR;
                }
            } else
                done = true;
            break;

        case 4: // WIN stuff: registry
#if __WINDOWS__
            if ( in_lib) {
                source = ( ftype == YAP_PL || ftype == YAP_QLY ? "library" : "startup" ) ;
                source = Yap_RegistryGetString( source );
                root = NULL;
            } else
#endif
                done = true;
            break;

        case 5: // search from the binary
        {
#ifndef __ANDROID__
            done = true;
            break;
#endif
            const char *pt = Yap_FindExecutable();

            if (pt) {
                source = ( ftype == YAP_SAVED_STATE || ftype == YAP_OBJ ? "../../lib/Yap" : "../../share/Yap" ) ;
                if (Yap_trueFileName(source, NULL, pt, save_buffer, access, ftype, expand_root, in_lib) )
                    root = save_buffer;
                else
                    done = true;
            } else {
                done = true;
            }
            source = ( isource ? isource : idef ) ;
        }
            break;
        case 6: // default, try current directory
            if (!isource && ftype == YAP_SAVED_STATE)
                source = idef;
            root = NULL;
            break;
        default:
            return false;
        }

        if (done)
            continue;
        //    { CACHE_REGS __android_log_print(ANDROID_LOG_ERROR,  __FUNCTION__, "root= %s %s ", root, source) ; }
        const char *work = expandWithPrefix( source, root );


        // expand names in case you have
        // to add a prefix
        if ( !access || exists( work ) )
            return true; // done
    }
    return false;
}

int
Yap_TrueFileName (const char *source, char *result, bool in_lib)
{
    return Yap_trueFileName (source, NULL, NULL, result, true, YAP_PL, true, in_lib);
}

int
Yap_TruePrefixedFileName (const char *source, const char *root, char *result, int in_lib)
{
    return Yap_trueFileName (source, NULL, root, result, true, YAP_PL, true, in_lib);
}

static Int
true_file_name ( USES_REGS1 )
{
    Term t = Deref(ARG1);

    if (IsVarTerm(t)) {
        Yap_Error(INSTANTIATION_ERROR,t,"argument to true_file_name unbound");
        return FALSE;
    }
    if (!IsAtomTerm(t)) {
        Yap_Error(TYPE_ERROR_ATOM,t,"argument to true_file_name");
        return FALSE;
    }
    if (!Yap_AbsoluteFileInBuffer( RepAtom(AtomOfTerm(t))->StrOfAE, LOCAL_FileNameBuf, YAP_FILENAME_MAX-1, true))
        return FALSE;
    return Yap_unify(ARG2, MkAtomTerm(Yap_LookupAtom(LOCAL_FileNameBuf)));
}

static Int
p_expand_file_name ( USES_REGS1 )
{
    Term t = Deref(ARG1);

    if (IsVarTerm(t)) {
        Yap_Error(INSTANTIATION_ERROR,t,"argument to true_file_name unbound");
        return FALSE;
    }
    if (!IsAtomTerm(t)) {
        Yap_Error(TYPE_ERROR_ATOM,t,"argument to true_file_name");
        return FALSE;
    }
    if (!Yap_trueFileName (RepAtom(AtomOfTerm(t))->StrOfAE, NULL, NULL, LOCAL_FileNameBuf, true, YAP_PL, true, false))
        return false;
    return Yap_unify(ARG2, MkAtomTerm(Yap_LookupAtom(LOCAL_FileNameBuf)));
}

static Int
true_file_name3 ( USES_REGS1 )
{
    Term t = Deref(ARG1), t2 = Deref(ARG2);
    char *root = NULL;

    if (IsVarTerm(t)) {
        Yap_Error(INSTANTIATION_ERROR,t,"argument to true_file_name unbound");
        return FALSE;
    }
    if (!IsAtomTerm(t)) {
        Yap_Error(TYPE_ERROR_ATOM,t,"argument to true_file_name");
        return FALSE;
    }
    if (!IsVarTerm(t2)) {
        if (!IsAtomTerm(t)) {
            Yap_Error(TYPE_ERROR_ATOM,t2,"argument to true_file_name");
            return FALSE;
        }
        root = RepAtom(AtomOfTerm(t2))->StrOfAE;
    }
    if (!Yap_trueFileName (RepAtom(AtomOfTerm(t))->StrOfAE, NULL, root, LOCAL_FileNameBuf, false, YAP_PL, false, false))
        return FALSE;
    return Yap_unify(ARG3, MkAtomTerm(Yap_LookupAtom(LOCAL_FileNameBuf)));
}

/* Executes $SHELL under Prolog */
/** @pred  sh


      Creates a new shell interaction.


  */
static Int
p_sh ( USES_REGS1 )
{				/* sh				 */
#ifdef HAVE_SYSTEM
    char *shell;
    shell = (char *) getenv ("SHELL");
    if (shell == NULL)
        shell = "/bin/sh";
    if (system (shell) < 0) {
#if HAVE_STRERROR
        Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil, "%s in sh/0", strerror(errno));
#else
        Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil, "in sh/0");
#endif
        return FALSE;
    }
    return TRUE;
#else
#ifdef MSH
    register char *shell;
    shell = "msh -i";
    system (shell);
    return (TRUE);
#else
    Yap_Error(SYSTEM_ERROR_INTERNAL,TermNil,"sh not available in this configuration");
    return(FALSE);
#endif /* MSH */
#endif
}

/** shell(+Command:text, -Status:integer) is det.

      Run an external command and wait for its completion.
  */
static Int
p_shell ( USES_REGS1 )
{				/* '$shell'(+SystCommand)			 */
    const char *cmd;
    Term t1 = Deref (ARG1);
    if (IsAtomTerm(t1))
        cmd = RepAtom(AtomOfTerm(t1))->StrOfAE;
    else if (IsStringTerm(t1))
        cmd = StringOfTerm(t1);
    else
        return FALSE;
#if _MSC_VER || defined(__MINGW32__)
    { int rval = system(cmd);

        return rval == 0;
    }

    return true;
#else
#if HAVE_SYSTEM
    char *shell;
    register int bourne = FALSE;

    shell = (char *) getenv ("SHELL");
    if (!strcmp (shell, "/bin/sh"))
        bourne = TRUE;
    if (shell == NIL)
        bourne = TRUE;
    /* Yap_CloseStreams(TRUE); */
    if (bourne)
        return system( cmd ) == 0;
    else {
        int status = -1;
        int child = fork ();

        if (child == 0) {			/* let the children go */
            if (!execl (shell, shell, "-c", cmd , NULL)) {
                exit(-1);
            }
            exit(TRUE);
        }
        {				/* put the father on wait */
            int result = child < 0 ||
                    /* vsc:I am not sure this is used, Stevens say wait returns an integer.
                 #if NO_UNION_WAIT
              */
                    wait ((&status)) != child ||
                    /*
                #else
                wait ((union wait *) (&status)) != child ||
                #endif
              */
                    status == 0;
            return result;
        }
    }
#else /* HAVE_SYSTEM */
#ifdef MSH
    register char *shell;
    shell = "msh -i";
    /* Yap_CloseStreams(); */
    system (shell);
    return TRUE;
#else
    Yap_Error (SYSTEM_ERROR_INTERNAL,TermNil,"shell not available in this configuration");
    return FALSE;
#endif
#endif /* HAVE_SYSTEM */
#endif /* _MSC_VER */
}

/** system(+Command:text).

      Run an external command.
  */

static Int
p_system ( USES_REGS1 )
{				/* '$system'(+SystCommand)	       */
    const char *cmd;
    Term t1 = Deref (ARG1);

    if (IsVarTerm(t1)) {
        Yap_Error(INSTANTIATION_ERROR,t1,"argument to system/1 unbound");
        return FALSE;
    } else if (IsAtomTerm(t1)) {
        cmd = RepAtom(AtomOfTerm(t1))->StrOfAE;
    } else if (IsStringTerm(t1)) {
        cmd = StringOfTerm(t1);
    } else {
        if (!Yap_GetName (LOCAL_FileNameBuf, YAP_FILENAME_MAX, t1)) {
            Yap_Error(TYPE_ERROR_ATOM,t1,"argument to system/1");
            return false;
        }
        cmd = LOCAL_FileNameBuf;
    }
    /* Yap_CloseStreams(TRUE); */
#if _MSC_VER || defined(__MINGW32__)

    { STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

        // Start the child process.
        if( !CreateProcess( NULL,   // No module name (use command line)
                            (LPSTR)cmd,            // Command line
                            NULL,           // Process handle not inheritable
                            NULL,           // Thread handle not inheritable
                            FALSE,          // Set handle inheritance to FALSE
                            0,              // No creation flags
                            NULL,           // Use parent's environment block
                            NULL,           // Use parent's starting directory
                            &si,            // Pointer to STARTUPINFO structure
                            &pi )           // Pointer to PROCESS_INFORMATION structure
                )
        {
            Yap_Error( SYSTEM_ERROR_INTERNAL, ARG1,  "CreateProcess failed (%d).\n", GetLastError() );
            return FALSE;
        }
        // Wait until child process exits.
        WaitForSingleObject( pi.hProcess, INFINITE );

        // Close process and thread handles.
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );

        return TRUE;
    }

    return FALSE;
#elif HAVE_SYSTEM
#if _MSC_VER
    _flushall();
#endif
    if (system (cmd)) {
#if HAVE_STRERROR
        Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM,t1,"%s in system(%s)", strerror(errno), cmd);
#else
        Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM,t1,"in system(%s)", cmd);
#endif
        return FALSE;
    }
    return TRUE;
#else
#ifdef MSH
    register char *shell;
    shell = "msh -i";
    /* Yap_CloseStreams(); */
    system (shell);
    return (TRUE);
#undef command
#else
    Yap_Error(SYSTEM_ERROR_INTERNAL,TermNil,"sh not available in this machine");
    return(FALSE);
#endif
#endif /* HAVE_SYSTEM */
}



static Int
p_mv ( USES_REGS1 )
{				/* rename(+OldName,+NewName)   */
#if HAVE_LINK
    int r;
    char *oldname, *newname;
    Term t1 = Deref (ARG1);
    Term t2 = Deref (ARG2);
    if (IsVarTerm(t1)) {
        Yap_Error(INSTANTIATION_ERROR, t1, "first argument to rename/2 unbound");
    } else if (!IsAtomTerm(t1)) {
        Yap_Error(TYPE_ERROR_ATOM, t1, "first argument to rename/2 not atom");
    }
    if (IsVarTerm(t2)) {
        Yap_Error(INSTANTIATION_ERROR, t2, "second argument to rename/2 unbound");
    } else if (!IsAtomTerm(t2)) {
        Yap_Error(TYPE_ERROR_ATOM, t2, "second argument to rename/2 not atom");
    } else {
      oldname = (RepAtom(AtomOfTerm(t1)))->StrOfAE;
      newname = (RepAtom(AtomOfTerm(t2)))->StrOfAE;    
      if ((r = link (oldname, newname)) == 0 && (r = unlink (oldname)) != 0)
        unlink (newname);
      if (r != 0) {
#if HAVE_STRERROR
        Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM,t2,"%s in rename(%s,%s)", strerror(errno),oldname,newname);
#else
        Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM,t2,"in rename(%s,%s)",oldname,newname);
#endif
        return false;
      }
      return true;
    }
#else
    Yap_Error(SYSTEM_ERROR_INTERNAL,TermNil,"rename/2 not available in this machine");
#endif
    return false;
}


#ifdef MAC

void
Yap_SetTextFile (name)
char *name;
{
#ifdef MACC
SetFileType (name, 'TEXT');
SetFileSignature (name, 'EDIT');
#else
FInfo f;
FInfo *p = &f;
GetFInfo (name, 0, p);
p->fdType = 'TEXT';
#ifdef MPW
if (mpwshell)
p->fdCreator = 'MPS\0';
#endif
#ifndef LIGHT
else
p->fdCreator = 'EDIT';
#endif
SetFInfo (name, 0, p);
#endif
}

#endif


/* return YAP's environment */
static Int p_getenv( USES_REGS1 )
{
#if HAVE_GETENV
    Term t1 = Deref(ARG1), to;
    char *s, *so;

    if (IsVarTerm(t1)) {
        Yap_Error(INSTANTIATION_ERROR, t1,
                  "first arg of getenv/2");
        return(FALSE);
    } else if (!IsAtomTerm(t1)) {
        Yap_Error(TYPE_ERROR_ATOM, t1,
                  "first arg of getenv/2");
        return(FALSE);
    } else s = RepAtom(AtomOfTerm(t1))->StrOfAE;
    if ((so = getenv(s)) == NULL)
        return(FALSE);
    to = MkAtomTerm(Yap_LookupAtom(so));
    return(Yap_unify_constant(ARG2,to));
#else
    Yap_Error(SYSTEM_ERROR_INTERNAL, TermNil,
              "getenv not available in this configuration");
    return (FALSE);
#endif
}

/* set a variable in YAP's environment */
static Int p_putenv( USES_REGS1 )
{
#if HAVE_PUTENV
    Term t1 = Deref(ARG1), t2 = Deref(ARG2);
    char *s, *s2, *p0, *p;

    if (IsVarTerm(t1)) {
        Yap_Error(INSTANTIATION_ERROR, t1,
                  "first arg to putenv/2");
        return(FALSE);
    } else if (!IsAtomTerm(t1)) {
        Yap_Error(TYPE_ERROR_ATOM, t1,
                  "first arg to putenv/2");
        return(FALSE);
    } else s = RepAtom(AtomOfTerm(t1))->StrOfAE;
    if (IsVarTerm(t2)) {
        Yap_Error(INSTANTIATION_ERROR, t1,
                  "second arg to putenv/2");
        return(FALSE);
    } else if (!IsAtomTerm(t2)) {
        Yap_Error(TYPE_ERROR_ATOM, t2,
                  "second arg to putenv/2");
        return(FALSE);
    } else s2 = RepAtom(AtomOfTerm(t2))->StrOfAE;
    while (!(p0 = p = Yap_AllocAtomSpace(strlen(s)+strlen(s2)+3))) {
        if (!Yap_growheap(FALSE, MinHeapGap, NULL)) {
            Yap_Error(RESOURCE_ERROR_HEAP, TermNil, LOCAL_ErrorMessage);
            return FALSE;
        }
    }
    while ((*p++ = *s++) != '\0');
    p[-1] = '=';
    while ((*p++ = *s2++) != '\0');
    if (putenv(p0) == 0)
        return TRUE;
#if HAVE_STRERROR
    Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil,
              "in putenv(%s)", strerror(errno), p0);
#else
    Yap_Error(SYSTEM_ERROR_OPERATING_SYSTEM, TermNil,
              "in putenv(%s)", p0);
#endif
    return FALSE;
#else
    Yap_Error(SYSTEM_ERROR_INTERNAL, TermNil,
              "putenv not available in this configuration");
    return FALSE;
#endif
}

static Int
p_host_type( USES_REGS1 ) {
    Term out = MkAtomTerm(Yap_LookupAtom(HOST_ALIAS));
    return(Yap_unify(out,ARG1));
}

static Int
p_yap_home( USES_REGS1 ) {
    Term out = MkAtomTerm(Yap_LookupAtom(YAP_ROOTDIR));
    return(Yap_unify(out,ARG1));
}

static Int
p_yap_paths( USES_REGS1 ) {
    Term out1, out2, out3;
    const char *env_destdir = getenv("DESTDIR");
    char destdir[YAP_FILENAME_MAX+1];

    if (env_destdir) {
        strncat(destdir, env_destdir, YAP_FILENAME_MAX );
        strncat(destdir, "/" YAP_LIBDIR, YAP_FILENAME_MAX );
        out1 = MkAtomTerm(Yap_LookupAtom(destdir));
    } else {
        out1 = MkAtomTerm(Yap_LookupAtom(YAP_LIBDIR));
    }
    if (env_destdir) {
        strncat(destdir, env_destdir, YAP_FILENAME_MAX );
        strncat(destdir, "/" YAP_SHAREDIR, YAP_FILENAME_MAX );
        out2 = MkAtomTerm(Yap_LookupAtom(destdir));
    } else {
        out2 = MkAtomTerm(Yap_LookupAtom(YAP_SHAREDIR));
    }
    if (env_destdir) {
        strncat(destdir, env_destdir, YAP_FILENAME_MAX );
        strncat(destdir, "/" YAP_BINDIR, YAP_FILENAME_MAX );
        out3 = MkAtomTerm(Yap_LookupAtom(destdir));
    } else {
        out3 = MkAtomTerm(Yap_LookupAtom(YAP_BINDIR));
    }
    return(Yap_unify(out1,ARG1) &&
           Yap_unify(out2,ARG2) &&
           Yap_unify(out3,ARG3));
}

static Int
p_log_event( USES_REGS1 ) {
    Term in = Deref(ARG1);
    Atom at;

    if (IsVarTerm(in))
        return FALSE;
    if (!IsAtomTerm(in))
        return FALSE;
    at = AtomOfTerm( in );
#if DEBUG
    if (IsWideAtom(at) )
        fprintf(stderr, "LOG %S\n", RepAtom(at)->WStrOfAE);
    else if (IsBlob(at))
        return FALSE;
    else
        fprintf(stderr, "LOG %s\n", RepAtom(at)->StrOfAE);
#endif
    if (IsWideAtom(at) || IsBlob(at))
        return FALSE;
    LOG(  " %s ",RepAtom(at)->StrOfAE);
    return TRUE;

}


static Int
p_env_separator( USES_REGS1 ) {
#if defined(_WIN32)
    return Yap_unify(MkIntegerTerm(';'),ARG1);
#else
    return Yap_unify(MkIntegerTerm(':'),ARG1);
#endif
}

/*
   * This is responsable for the initialization of all machine dependant
   * predicates
   */
void
Yap_InitSysbits (int wid)
{
    CACHE_REGS
        #if  __simplescalar__
    {
        char *pwd = getenv("PWD");
        strncpy(GLOBAL_pwd,pwd,YAP_FILENAME_MAX);
    }
#endif
    Yap_InitWTime ();
    Yap_InitRandom ();
    /* let the caller control signals as it sees fit */
    Yap_InitOSSignals (worker_id);
}

static Int
p_unix( USES_REGS1 )
{
#ifdef unix
    return TRUE;
#else
#ifdef __unix__
    return TRUE;
#else
#ifdef __APPLE__
    return TRUE;
#else
    return FALSE;
#endif
#endif
#endif
}

static Int
p_win32( USES_REGS1 )
{
#ifdef _WIN32
    return TRUE;
#else
#ifdef __CYGWIN__
    return TRUE;
#else
    return FALSE;
#endif
#endif
}


static Int
p_ld_path( USES_REGS1 )
{
    return Yap_unify(ARG1,MkAtomTerm(Yap_LookupAtom(YAP_LIBDIR)));
}

static Int
p_address_bits( USES_REGS1 )
{
#if SIZEOF_INT_P==4
    return Yap_unify(ARG1,MkIntTerm(32));
#else
    return Yap_unify(ARG1,MkIntTerm(64));
#endif
}



#ifdef _WIN32

/* This code is from SWI-Prolog by Jan Wielemaker */

#define wstreq(s,q) (wcscmp((s), (q)) == 0)

static HKEY
reg_open_key(const wchar_t *which, int create)
{ HKEY key = HKEY_CURRENT_USER;
    DWORD disp;
    LONG rval;

    while(*which)
    { wchar_t buf[256];
        wchar_t *s;
        HKEY tmp;

        for(s=buf; *which && !(*which == '/' || *which == '\\'); )
            *s++ = *which++;
        *s = '\0';
        if ( *which )
            which++;

        if ( wstreq(buf, L"HKEY_CLASSES_ROOT") )
        { key = HKEY_CLASSES_ROOT;
            continue;
        } else if ( wstreq(buf, L"HKEY_CURRENT_USER") )
        { key = HKEY_CURRENT_USER;
            continue;
        } else if ( wstreq(buf, L"HKEY_LOCAL_MACHINE") )
        { key = HKEY_LOCAL_MACHINE;
            continue;
        } else if ( wstreq(buf, L"HKEY_USERS") )
        { key = HKEY_USERS;
            continue;
        }

        if ( RegOpenKeyExW(key, buf, 0L, KEY_READ, &tmp) == ERROR_SUCCESS )
        { RegCloseKey(key);
            key = tmp;
            continue;
        }

        if ( !create )
            return NULL;

        rval = RegCreateKeyExW(key, buf, 0, L"", 0,
                               KEY_ALL_ACCESS, NULL, &tmp, &disp);
        RegCloseKey(key);
        if ( rval == ERROR_SUCCESS )
            key = tmp;
        else
            return NULL;
    }

    return key;
}

#define MAXREGSTRLEN 1024

static void
recover_space(wchar_t *k, Atom At)
{
    if (At->WStrOfAE != k)
        Yap_FreeCodeSpace((char *)k);
}

static wchar_t *
WideStringFromAtom(Atom KeyAt USES_REGS)
{
    if (IsWideAtom(KeyAt)) {
        return KeyAt->WStrOfAE;
    } else {
        int len = strlen(KeyAt->StrOfAE);
        int sz = sizeof(wchar_t)*(len+1);
        char *chp = KeyAt->StrOfAE;
        wchar_t *kptr, *k;

        k = (wchar_t *)Yap_AllocCodeSpace(sz);
        while (k == NULL) {
            if (!Yap_growheap(FALSE, sz, NULL)) {
                Yap_Error(RESOURCE_ERROR_HEAP, MkIntegerTerm(sz), "generating key in win_registry_get_value/3");
                return FALSE;
            }
        }
        kptr = k;
        while ((*kptr++ = *chp++));
        return k;
    }
}

static Int
p_win_registry_get_value( USES_REGS1 )
{
    DWORD type;
    BYTE  data[MAXREGSTRLEN];
    DWORD len = sizeof(data);
    wchar_t *k, *name;
    HKEY key;
    Term Key = Deref(ARG1);
    Term Name = Deref(ARG2);
    Atom KeyAt, NameAt;

    if (IsVarTerm(Key)) {
        Yap_Error(INSTANTIATION_ERROR,Key,"argument to win_registry_get_value unbound");
        return FALSE;
    }
    if (!IsAtomTerm(Key)) {
        Yap_Error(TYPE_ERROR_ATOM,Key,"argument to win_registry_get_value");
        return FALSE;
    }
    KeyAt = AtomOfTerm(Key);
    if (IsVarTerm(Name)) {
        Yap_Error(INSTANTIATION_ERROR,Key,"argument to win_registry_get_value unbound");
        return FALSE;
    }
    if (!IsAtomTerm(Name)) {
        Yap_Error(TYPE_ERROR_ATOM,Key,"argument to win_registry_get_value");
        return FALSE;
    }
    NameAt = AtomOfTerm(Name);

    k = WideStringFromAtom(KeyAt PASS_REGS);
    if ( !(key=reg_open_key(k, FALSE)) ) {
        Yap_Error(EXISTENCE_ERROR_KEY, Key, "argument to win_registry_get_value");
        recover_space(k, KeyAt);
        return FALSE;
    }
    name = WideStringFromAtom(NameAt PASS_REGS);

    if ( RegQueryValueExW(key, name, NULL, &type, data, &len) == ERROR_SUCCESS ) {
        RegCloseKey(key);
        switch(type) {
        case REG_SZ:
            recover_space(k, KeyAt);
            recover_space(name, NameAt);
            ((wchar_t *)data)[len] = '\0';
            return Yap_unify(MkAtomTerm(Yap_LookupMaybeWideAtom((wchar_t *)data)),ARG3);
        case REG_DWORD:
            recover_space(k, KeyAt);
            recover_space(name, NameAt);
        {
            DWORD *d = (DWORD *)data;
            return Yap_unify(MkIntegerTerm((Int)d[0]),ARG3);
        }
        default:
            recover_space(k, KeyAt);
            recover_space(name, NameAt);
            return FALSE;
        }
    }
    recover_space(k, KeyAt);
    recover_space(name, NameAt);
    return FALSE;
}

char *
Yap_RegistryGetString(char *name)
{
    DWORD type;
    BYTE  data[MAXREGSTRLEN];
    DWORD len = sizeof(data);
    HKEY key;
    char *ptr;
    int i;

#if SIZEOF_INT_P == 8
    if ( !(key=reg_open_key(L"HKEY_LOCAL_MACHINE/SOFTWARE/YAP/Prolog64", FALSE)) ) {
        return NULL;
    }
#else
    if ( !(key=reg_open_key(L"HKEY_LOCAL_MACHINE/SOFTWARE/YAP/Prolog", FALSE)) ) {
        return NULL;
    }
#endif
    if ( RegQueryValueEx(key, name, NULL, &type, data, &len) == ERROR_SUCCESS ) {
        RegCloseKey(key);
        switch(type) {
        case REG_SZ:
            ptr = malloc(len+2);
            if (!ptr)
                return NULL;
            for (i=0; i<= len; i++)
                ptr[i] = data[i];
            ptr[len+1] = '\0';
            return ptr;
        default:
            return NULL;
        }
    }
    return NULL;
}


#endif

void
Yap_InitSysPreds(void)
{
    Yap_InitCPred ("log_event", 1, p_log_event, SafePredFlag|SyncPredFlag);
    Yap_InitCPred ("sh", 0, p_sh, SafePredFlag|SyncPredFlag);
    Yap_InitCPred ("$shell", 1, p_shell, SafePredFlag|SyncPredFlag);
    Yap_InitCPred ("system", 1, p_system, SafePredFlag|SyncPredFlag|UserCPredFlag);
    Yap_InitCPred ("$rename", 2, p_mv, SafePredFlag|SyncPredFlag);
    Yap_InitCPred ("$yap_home", 1, p_yap_home, SafePredFlag);
    Yap_InitCPred ("$yap_paths", 3, p_yap_paths, SafePredFlag);
    Yap_InitCPred ("$dir_separator", 1, p_dir_sp, SafePredFlag);
    Yap_InitCPred ("libraries_directories",2, libraries_directories, 0);
    Yap_InitCPred ("system_library", 1, system_library, 0);
    Yap_InitCPred ("commons_library", 1, commons_library, 0);
    Yap_InitCPred ("$getenv", 2, p_getenv, SafePredFlag);
    Yap_InitCPred ("$putenv", 2, p_putenv, SafePredFlag|SyncPredFlag);
    Yap_InitCPred ("$host_type", 1, p_host_type, SafePredFlag|SyncPredFlag);
    Yap_InitCPred ("$env_separator", 1, p_env_separator, SafePredFlag);
    Yap_InitCPred ("$unix", 0, p_unix, SafePredFlag);
    Yap_InitCPred ("$win32", 0, p_win32, SafePredFlag);
    Yap_InitCPred ("$ld_path", 1, p_ld_path, SafePredFlag);
    Yap_InitCPred ("$address_bits", 1, p_address_bits, SafePredFlag);
    Yap_InitCPred ("$expand_file_name", 2, p_expand_file_name, SyncPredFlag);
    Yap_InitCPred ("expand_file_name", 3, expand_file_name3, SyncPredFlag);
    Yap_InitCPred ("expand_file_name", 2, expand_file_name, SyncPredFlag);
    Yap_InitCPred ("working_directory", 2,working_directory, SyncPredFlag);
    Yap_InitCPred ("prolog_to_os_filename", 2, prolog_to_os_filename, SyncPredFlag);
    Yap_InitCPred ("prolog_to_os_filename", 2, prolog_to_os_filename, SyncPredFlag);
#ifdef _WIN32
    Yap_InitCPred ("win_registry_get_value", 3, p_win_registry_get_value,0);
#endif
    Yap_InitCPred ("absolute_file_system_path", 2, absolute_file_system_path, 0);
    Yap_InitCPred ("real_path", 2, prolog_realpath, 0);
    Yap_InitCPred ("true_file_name", 2,
                   true_file_name, SyncPredFlag);
    Yap_InitCPred ("true_file_name", 3, true_file_name3, SyncPredFlag);
    Yap_InitCPred ("rmdir", 2, p_rmdir, SyncPredFlag);
    Yap_InitCPred ("make_directory", 1, make_directory, SyncPredFlag);
}

