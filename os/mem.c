/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		sockets.c *
* Last rev:	5/2/88							 *
* mods:									 *
* comments:	Input/Output C implemented predicates			 *
*									 *
*************************************************************************/
#ifdef SCCS
static char SccsId[] = "%W% %G%";
#endif

/*
 * This file includes the definition of a socket related IO.
 *
 */

#include "sysbits.h"

#if !MAY_READ
static int MemGetc(int);

/* read from memory */
static int MemGetc(int sno) {
  register StreamDesc *s = &GLOBAL_Stream[sno];
  Int ch;
  int spos;

  spos = s->u.mem_string.pos;
  if (spos == s->u.mem_string.max_size) {
    return -1;
  } else {
    ch = s->u.mem_string.buf[spos];
    s->u.mem_string.pos = ++spos;
  }
  return ch;
}
#endif

#if !MAY_WRITE
static int MemPutc(int, int);

/* static */
static int MemPutc(int sno, int ch) {
  StreamDesc *s = &GLOBAL_Stream[sno];
#if MAC || _MSC_VER
  if (ch == 10) {
    ch = '\n';
  }
#endif
  s->u.mem_string.buf[s->u.mem_string.pos++] = ch;
  if (s->u.mem_string.pos >= s->u.mem_string.max_size - 8) {
    int old_src = s->u.mem_string.src, new_src;

    /* oops, we have reached an overflow */
    Int new_max_size = s->u.mem_string.max_size + Yap_page_size;
    char *newbuf;

    if (old_src == MEM_BUF_CODE &&
        (newbuf = Yap_AllocAtomSpace(new_max_size * sizeof(char))) != NULL) {
      new_src = MEM_BUF_CODE;
#if HAVE_MEMMOVE
      memmove((void *)newbuf, (void *)s->u.mem_string.buf,
              (size_t)((s->u.mem_string.pos) * sizeof(char)));
#else
      {
        Int n = s->u.mem_string.pos;
        char *to = newbuf;
        char *from = s->u.mem_string.buf;
        while (n-- >= 0) {
          *to++ = *from++;
        }
      }
#endif
      Yap_FreeAtomSpace(s->u.mem_string.buf);
#if !HAVE_SYSTEM_MALLOC
    } else if ((newbuf = (ADDR)realloc(s->u.mem_string.buf,
                                       new_max_size * sizeof(char))) != NULL) {
      new_src = MEM_BUF_MALLOC;
#endif
    } else {
      if (GLOBAL_Stream[sno].u.mem_string.error_handler) {
        CACHE_REGS
        LOCAL_Error_Size = new_max_size * sizeof(char);
        save_machine_regs();
        longjmp(*(jmp_buf *)GLOBAL_Stream[sno].u.mem_string.error_handler, 1);
      } else {
        Yap_Error(RESOURCE_ERROR_HEAP, TermNil,
                  "YAP could not grow heap for writing to string");
      }
      return -1;
    }
    if (old_src == MEM_BUF_CODE) {
    }
    s->u.mem_string.buf = newbuf;
    s->u.mem_string.max_size = new_max_size;
    s->u.mem_string.src = new_src;
  }
  count_output_char(ch, s);
  return ((int)ch);
}

#endif

int Yap_open_buf_read_stream(const char *buf, size_t nchars, encoding_t *encp,
                             memBufSource src) {
  CACHE_REGS
  int sno;
  StreamDesc *st;
  FILE *f;
  encoding_t encoding;
  stream_flags_t flags;

  sno = GetFreeStreamD();
  if (sno < 0)
    return (PlIOError(RESOURCE_ERROR_MAX_STREAMS, TermNil,
                      "new stream not available for open_mem_read_stream/1"));
  st = GLOBAL_Stream + sno;
  if (encp)
    encoding = *encp;
  else
    encoding = LOCAL_encoding;
#if MAY_READ
  // like any file stream.
  st->file = f = fmemopen((void *)buf, nchars, "r");
  flags = Input_Stream_f | InMemory_Stream_f | Seekable_Stream_f;
#else
  st->file = f = NULL;
  flags = Input_Stream_f | InMemory_Stream_f;
#endif
  Yap_initStream(sno, f, NULL, TermNil, encoding, flags, AtomRead);
// like any file stream.
#if !MAY_READ
  /* currently these streams are not seekable */
  st->status = Input_Stream_f | InMemory_Stream_f;
  st->u.mem_string.pos = 0;
  st->u.mem_string.buf = (char *)buf;
  st->u.mem_string.max_size = nchars;
  st->u.mem_string.error_handler = NULL;
  st->u.mem_string.src = src;
#endif
  Yap_DefaultStreamOps(st);
  UNLOCK(st->streamlock);
  return sno;
}

static Int
    open_mem_read_stream(USES_REGS1) /* $open_mem_read_stream(+List,-Stream) */
{
  Term t, ti;
  int sno;
  Int sl = 0, nchars = 0;
  char *buf;

  ti = Deref(ARG1);
  while (ti != TermNil) {
    if (IsVarTerm(ti)) {
      Yap_Error(INSTANTIATION_ERROR, ti, "open_mem_read_stream");
      return (FALSE);
    } else if (!IsPairTerm(ti)) {
      Yap_Error(TYPE_ERROR_LIST, ti, "open_mem_read_stream");
      return (FALSE);
    } else {
      sl++;
      ti = TailOfTerm(ti);
    }
  }
  while ((buf = (char *)Yap_AllocAtomSpace((sl + 1) * sizeof(char))) == NULL) {
    if (!Yap_growheap(FALSE, (sl + 1) * sizeof(char), NULL)) {
      Yap_Error(RESOURCE_ERROR_HEAP, TermNil, LOCAL_ErrorMessage);
      return (FALSE);
    }
  }
  ti = Deref(ARG1);
  while (ti != TermNil) {
    Term ts = HeadOfTerm(ti);

    if (IsVarTerm(ts)) {
      Yap_Error(INSTANTIATION_ERROR, ARG1, "open_mem_read_stream");
      return (FALSE);
    } else if (!IsIntTerm(ts)) {
      Yap_Error(TYPE_ERROR_INTEGER, ARG1, "open_mem_read_stream");
      return (FALSE);
    }
    buf[nchars++] = IntOfTerm(ts);
    ti = TailOfTerm(ti);
  }
  buf[nchars] = '\0';
  sno = Yap_open_buf_read_stream(buf, nchars, &LOCAL_encoding, MEM_BUF_CODE);
  t = Yap_MkStream(sno);
  return (Yap_unify(ARG2, t));
}

// open a buffer for writing, currently just ignores buf and nchars.

int Yap_open_buf_write_stream(encoding_t enc, memBufSource src) {
  CACHE_REGS
  int sno;
  StreamDesc *st;

  sno = GetFreeStreamD();
  if (sno < 0)
    return -1;
  st = GLOBAL_Stream + sno;
  st->status = Output_Stream_f | InMemory_Stream_f;
  st->linepos = 0;
  st->charcount = 0;
  st->linecount = 1;
  st->encoding = enc;
  Yap_DefaultStreamOps(st);
#if MAY_WRITE
#if HAVE_OPEN_MEMSTREAM
  st->file = open_memstream(&st->nbuf, &st->nsize);
  st->status |= Seekable_Stream_f;
#else
  st->file = fmemopen((void *)buf, nchars, "w");
  st->nsize = nchars;
  st->nbuf = buf;
  if (!st->nbuf) {
    return -1;
  }
#endif
#else
  char buf[YAP_FILENAME_MAX + 1];
  st->nbuf = buf;
  st->u.mem_string.buf = buf;
  st->u.mem_string.max_size = YAP_FILENAME_MAX;
#endif
  UNLOCK(st->streamlock);
  return sno;
}

int Yap_OpenBufWriteStream(USES_REGS1) {

  return Yap_open_buf_write_stream(
      GLOBAL_Stream[LOCAL_c_output_stream].encoding, 0);
}

static Int
    open_mem_write_stream(USES_REGS1) /* $open_mem_write_stream(-Stream) */
{
  Term t;
  int sno;

  sno = Yap_OpenBufWriteStream(PASS_REGS1);
  if (sno == -1)
    return (PlIOError(SYSTEM_ERROR_INTERNAL, TermNil,
                      "new stream not available for open_mem_read_stream/1"));
  t = Yap_MkStream(sno);
  return (Yap_unify(ARG1, t));
}

/**
 * Yap_PeekMemwriteStream() shows the current buffer for a memory stream.
 *
 * @param sno, the in-memory stream
 *
 * @return temporary buffer, discarded by close and may be moved away
 * by other writes..
 */
char *Yap_MemExportStreamPtr(int sno) {
#if MAY_WRITE
  char *s;
  if (fflush(GLOBAL_Stream[sno].file) == 0) {
    s = GLOBAL_Stream[sno].nbuf;
    return s;
  }
  return NULL;
#else
  return GLOBAL_Stream[sno].u.mem_string.buf;
#endif
}

static Int peek_mem_write_stream(
    USES_REGS1) { /* '$peek_mem_write_stream'(+GLOBAL_Stream,?S0,?S) */
  Int sno =
      Yap_CheckStream(ARG1, (Output_Stream_f | InMemory_Stream_f), "close/2");
  Int i;
  Term tf = ARG2;
  CELL *HI;
  const char *ptr;

  if (sno < 0)
    return (FALSE);
restart:
  HI = HR;
#if MAY_WRITE
  if (fflush(GLOBAL_Stream[sno].file) == 0) {
    ptr = GLOBAL_Stream[sno].nbuf;
    i = fseek(GLOBAL_Stream[sno].file, SEEK_END, 0);
  }
#else
  ptr = GLOBAL_Stream[sno].u.mem_string.buf;
  i = GLOBAL_Stream[sno].u.mem_string.pos;
#endif
  while (i > 0) {
    --i;
    tf = MkPairTerm(MkIntTerm(ptr[i]), tf);
    if (HR + 1024 >= ASP) {
      UNLOCK(GLOBAL_Stream[sno].streamlock);
      HR = HI;
      if (!Yap_gcl((ASP - HI) * sizeof(CELL), 3, ENV, Yap_gcP())) {
        UNLOCK(GLOBAL_Stream[sno].streamlock);
        Yap_Error(RESOURCE_ERROR_STACK, TermNil, LOCAL_ErrorMessage);
        return (FALSE);
      }
      i = GLOBAL_Stream[sno].u.mem_string.pos;
      tf = ARG2;
      LOCK(GLOBAL_Stream[sno].streamlock);
      goto restart;
    }
  }
  UNLOCK(GLOBAL_Stream[sno].streamlock);
  return (Yap_unify(ARG3, tf));
}

void Yap_MemOps(StreamDesc *st) {
#if MAY_WRITE
  st->stream_putc = FilePutc;
#else
  st->stream_putc = MemPutc;
#endif

#if MAY_READ
  st->stream_getc = PlGetc;
#else
  st->stream_getc = MemGetc;
#endif
}

bool Yap_CloseMemoryStream(int sno) {
  if (!(GLOBAL_Stream[sno].status & Output_Stream_f)) {
#if MAY_WRITE
    fflush(GLOBAL_Stream[sno].file);
    fclose(GLOBAL_Stream[sno].file);
    if (GLOBAL_Stream[sno].status & FreeOnClose_Stream_f)
      free(GLOBAL_Stream[sno].nbuf);
#else
    if (GLOBAL_Stream[sno].u.mem_string.src == MEM_BUF_CODE)
      Yap_FreeAtomSpace(GLOBAL_Stream[sno].u.mem_string.buf);
    else if (GLOBAL_Stream[sno].u.mem_string.src == MEM_BUF_MALLOC) {
      free(GLOBAL_Stream[sno].u.mem_string.buf);
    }
#endif
  } else {
#if MAY_READ
    fclose(GLOBAL_Stream[sno].file);
    if (GLOBAL_Stream[sno].status & FreeOnClose_Stream_f)
      free(GLOBAL_Stream[sno].nbuf);
#else
    if (GLOBAL_Stream[sno].u.mem_string.src == MEM_BUF_CODE)
      Yap_FreeAtomSpace(GLOBAL_Stream[sno].u.mem_string.buf);
    else if (GLOBAL_Stream[sno].u.mem_string.src == MEM_BUF_MALLOC) {
      free(GLOBAL_Stream[sno].u.mem_string.buf);
    }
#endif
  }
  return true;
}

void Yap_InitMems(void) {
  CACHE_REGS
  Term cm = CurrentModule;
  CurrentModule = CHARSIO_MODULE;
  Yap_InitCPred("open_mem_read_stream", 2, open_mem_read_stream, SyncPredFlag);
  Yap_InitCPred("open_mem_write_stream", 1, open_mem_write_stream,
                SyncPredFlag);
  Yap_InitCPred("peek_mem_write_stream", 3, peek_mem_write_stream,
                SyncPredFlag);
  CurrentModule = cm;
}
