/*************************************************************************
*									 *
*	 YAP Prolog 	@(#)c_interface.h	2.2			 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		yap_structs.h						 *
* Last rev:	15/5/2000						 *
* mods:									 *
* comments:	Data structures and defines used in the Interface	 *
*									 *
*************************************************************************/

#ifndef _YAPDEFS_H

#define _YAPDEFS_H 1

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef YAP_H

/* The YAP main types */
#include "YapTerm.h"

/**
  This term can never be constructed as a valid term, so it is
  used as a "BAD" term
*/
#define TermZERO ((Term)0)

#else

#include "YapConfig.h"

#endif /* YAP_H */

/* truth-values */
/* stdbool defines the booleam type, bool,
   and the constants false and true */
#if HAVE_STDINT_H
#include <stdint.h>
#endif
#if HAVE_STDBOOL_H
#include <stdbool.h>
#else
#ifndef true
typedef int _Bool;
v
#define bool _Bool;

#define false 0
#define true 1
#endif
#endif /* HAVE_STDBOOL_H */

/**
   FALSE and TRUE are the pre-standard versions,
   still widely used.
*/
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif
#ifndef YAP_Bool
typedef bool YAP_Bool;
#endif

#ifdef YAP_H

/* if Yap.h is available, just reexport */

#define YAP_CELL CELL

#define YAP_Term Term

#define YAP_Arity arity_t

#define YAP_Module Term

#define YAP_Functor Functor

#define YAP_Atom Atom

#define YAP_Int Int

#define YAP_UInt UInt

#define YAP_Float Float

#define YAP_handle_t yhandle_t

#define YAP_PredEntryPtr struct pred_entry *

#define YAP_UserCPred CPredicate

#define YAP_agc_hook Agc_hook

#define YAP_encoding_t encoding_t

#else

/* Type definitions */

#if _WIN64
    typedef unsigned long long YAP_CELL;
#else
typedef uintptr_t YAP_CELL;
#endif

typedef YAP_CELL YAP_Term;

typedef YAP_CELL YAP_Arity;

typedef YAP_Term YAP_Module;

typedef struct FunctorEntry *YAP_Functor;

typedef struct AtomEntry *YAP_Atom;

#if _WIN64
typedef long long int YAP_Int;

typedef unsigned long long int YAP_UInt;

#else
typedef long int YAP_Int;

typedef unsigned long int YAP_UInt;
#endif

typedef double YAP_Float;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef YAP_Int YAP_handle_t;

typedef struct YAP_pred_entry *YAP_PredEntryPtr;

typedef YAP_Bool (*YAP_UserCPred)(void);

typedef int (*YAP_agc_hook)(void *_Atom);

#include "YapError.h"

#include "../os/encoding.h"

typedef encoding_t YAP_encoding_t;

#endif

typedef struct YAP_thread_attr_struct {
  size_t ssize;
  size_t tsize;
  size_t sysize;
  int (*cancel)(int thread);
  YAP_Term egoal, alias;
} YAP_thread_attr;

#ifdef YAP_H
#include <threads.h>
#endif

typedef enum { YAP_BIN = 0x0001,
	       YAP_TEXT = 0x0002,
	       YAP_SAVED_STATE = 0x0004,
	       YAP_OBJ = 0x0008,
	       YAP_PL = 0x0010,
	       YAP_BOOT_PL = 0x0030,
	       YAP_QLY = 0x0040,
	       YAP_EXE = 0x0080
} YAP_file_type_t;

#define YAP_ANY_FILE (0x00ff)

typedef enum {
  YAP_TAG_ATT = 0x1,
  YAP_TAG_UNBOUND = 0x2,
  YAP_TAG_REF = 0x4,
  YAP_TAG_PAIR = 0x8,
  YAP_TAG_ATOM = 0x10,
  YAP_TAG_INT = 0x20,
  YAP_TAG_LONG_INT = 0x40,
  YAP_TAG_BIG_INT = 0x80,
  YAP_TAG_RATIONAL = 0x100,
  YAP_TAG_FLOAT = 0x200,
  YAP_TAG_OPAQUE = 0x400,
  YAP_TAG_APPL = 0x800,
    YAP_TAG_DBREF = 0x1000,
    YAP_TAG_STRING = 0x2000,
    YAP_TAG_ARRAY = 0x4000
} YAP_tag_t;

#define YAP_BOOT_FROM_SAVED_CODE 1
#define YAP_BOOT_FROM_SAVED_STACKS 2
#define YAP_BOOT_ERROR -1

#define YAP_WRITE_QUOTED 1
#define YAP_WRITE_IGNORE_OPS 2
#define YAP_WRITE_HANDLE_VARS 4
#define YAP_WRITE_USE_PORTRAY 8
#define YAP_WRITE_HANDLE_CYCLES 0x20
#define YAP_WRITE_BACKQUOTE_STRING 0x80
#define YAP_WRITE_ATTVAR_NONE 0x100
#define YAP_WRITE_ATTVAR_DOTS 0x200
#define YAP_WRITE_ATTVAR_PORTRAY 0x400
#define YAP_WRITE_BLOB_PORTRAY 0x800

#define YAP_CONSULT_MODE 0
#define YAP_RECONSULT_MODE 1
#define YAP_BOOT_MODE 2

typedef struct yap_boot_params {
  /* boot type as suggested by the user */
  YAP_file_type_t initial_file_type;
  /* if NON-NULL, path where we can find the saved state */
  const char *SavedState;
  /* if NON-0, minimal size for Heap or Code Area */
  unsigned long int HeapSize;
  /* if NON-0, maximal size for Heap or Code Area */
  unsigned long int MaxHeapSize;
  /* if NON-0, minimal size for Local+Global Stack */
  unsigned long int StackSize;
  /* if NON-0, maximal size for Local+Global Stack */
  unsigned long int MaxStackSize;
  unsigned long int MaxGlobalSize;
  /* if NON-0, minimal size for Trail */
  unsigned long int TrailSize;
  /* if NON-0, maximal size for Trail */
  unsigned long int MaxTrailSize;
  /* if NON-0, minimal size for AttributeVarStack */
  unsigned long int AttsSize;
  /* if NON-0, maximal size for AttributeVarStack */
  unsigned long int MaxAttsSize;
  /* if NON-NULL, value for YAPLIBDIR */
  const char *YapLibDir;
  /* if NON-NULL, name for a Prolog file to use when booting  */
  const char *YapPrologBootFile;
  /* if NON-NULL, name for a Prolog file to use when initializing  */
  const char *YapPrologInitGoal;
  /* if NON-NULL, name for a Prolog file to consult before entering top-level */
  const char *YapPrologRCFile;
  /* if NON-NULL, a goal to run before top-level  */
  const char *YapPrologGoal;
  /* if NON-NULL, a goal to run as top-level  */
  const char *YapPrologTopLevelGoal;
  /* if NON-NULL, a path to extend file-search-path   */
  const char *YapPrologAddPath;
  /* if previous NON-NULL and TRUE, halt after consulting that file  */
  bool HaltAfterConsult;
  /* ignore .yaprc, .prolog.ini, etc. files.  */
  bool FastBoot;
  /* the next field only interest YAPTAB */
  /* if NON-0, maximum size for Table Space */
  unsigned long int MaxTableSpaceSize;
  /* the next three fields only interest YAPOR, but we keep them so that
     users don't need to recompile DLL in order to use YAPOR */
  /* if NON-0, number of workers we want to have (default=1) */
  unsigned long int NumberWorkers;
  /* if NON-0, manage the inner scheduler loop (default = 10) */
  unsigned long int SchedulerLoop;
  /* if NON-0, say how long to keep nodes (default = 3) */
  unsigned long int DelayedReleaseLoad;
  /* end of YAPOR fields */
  /* whether Prolog should handle interrupts */
  int PrologShouldHandleInterrupts;
  /* flag for JIT mode */
  int ExecutionMode;
  /* number of arguments that Prolog will see */
  int Argc;
  /* array of arguments as seen by Prolog */
  char **Argv;
  /* QuietMode */
  int QuietMode;

/* nf: Begin ypp preprocessor code */
#define YAP_MAX_YPP_DEFS 100
  char *def_var[YAP_MAX_YPP_DEFS];
  char *def_value[YAP_MAX_YPP_DEFS];
  int def_c;
/* End preprocessor code */

#ifdef MYDDAS_MYSQL
  /* If any myddas option was given */
  short myddas;
  /* MYDDAS Fields */
  char *myddas_user;
  char *myddas_pass;
  char *myddas_db;
  char *myddas_host;
#endif
  /* errornumber */
  int ErrorNo;
  /* errorstring */
  char *ErrorCause;
} YAP_init_args;

#ifdef YAP_H
Int Yap_InitDefaults(YAP_init_args *init_args, char saved_state[]);
#endif

/* this should be opaque to the user */
typedef struct {
  unsigned long b;      //> choice-point at entry
  YAP_handle_t CurSlot; //> variables at entry
  YAP_handle_t EndSlot; //> variables at successful execution
  struct yami *p;       //> Program Counter at entry
  struct yami *cp;      //> Continuation PC at entry
} YAP_dogoalinfo;

// query manipulation support

typedef struct open_query_struct {
  int q_open;
  int q_state;
  YAP_handle_t q_g;
  struct pred_entry *q_pe;
  struct yami *q_p, *q_cp;
  jmp_buf q_env;
  int q_flags;
  YAP_dogoalinfo q_h;
  struct open_query_struct *oq;
} YAP_openQuery;

typedef void (*YAP_halt_hook)(int exit_code, void *closure);

typedef YAP_Int YAP_opaque_tag_t;

typedef YAP_Bool (*YAP_Opaque_CallOnFail)(void *);
typedef YAP_Bool (*YAP_Opaque_CallOnWrite)(FILE *, YAP_opaque_tag_t, void *,
                                           int);
typedef YAP_Int (*YAP_Opaque_CallOnGCMark)(YAP_opaque_tag_t, void *, YAP_Term *,
                                           YAP_Int);
typedef YAP_Bool (*YAP_Opaque_CallOnGCRelocate)(YAP_opaque_tag_t, void *,
                                                YAP_Term *, YAP_Int);

typedef struct YAP_opaque_handler_struct {
  YAP_Opaque_CallOnFail fail_handler;
  YAP_Opaque_CallOnWrite write_handler;
  YAP_Opaque_CallOnGCMark mark_handler;
  YAP_Opaque_CallOnGCRelocate relocate_handler;
} YAP_opaque_handler_t;

/********* execution mode ***********************/

typedef enum {
  YAPC_INTERPRETED,     /* interpreted */
  YAPC_MIXED_MODE_USER, /* mixed mode only for user predicates */
  YAPC_MIXED_MODE_ALL,  /* mixed mode for all predicates */
  YAPC_COMPILE_USER,    /* compile all user predicates*/
  YAPC_COMPILE_ALL      /* compile all predicates */
} yapc_exec_mode;

/** Stream Modes: */
typedef enum stream_f {
  Free_Stream_f = 0x000001,   /**< Free YAP Stream */
  Input_Stream_f = 0x000002,  /**< Input Stream */
  Output_Stream_f = 0x000004, /**< Output Stream in Truncate Mode */
  Append_Stream_f = 0x000008, /**< Output Stream in Append Mod */
  Eof_Stream_f = 0x000010,    /**< Stream found an EOF */
  Null_Stream_f = 0x000020,   /**< Stream is /dev/null, or equivant */
  Tty_Stream_f = 0x000040,    /**< Stream is a terminal */
  Socket_Stream_f = 0x000080, /**< Socket Stream */
  Binary_Stream_f = 0x000100, /**< Stream is not eof */
  Eof_Error_Stream_f =
      0x000200, /**< Stream should generate error on trying to read after EOF */
  Reset_Eof_Stream_f =
      0x000400, /**< Stream should be reset on findind an EO (C-D and console.*/
  Past_Eof_Stream_f = 0x000800, /**< Read EOF from stream */
  Push_Eof_Stream_f = 0x001000, /**< keep on sending EOFs */
  Seekable_Stream_f =
      0x002000, /**< we can jump around the stream (std regular files) */
  Promptable_Stream_f = 0x004000,    /**< Interactive line-by-line stream */
  Client_Socket_Stream_f = 0x008000, /**< socket in client mode */
  Server_Socket_Stream_f = 0x010000, /**< socket in server mode */
  InMemory_Stream_f = 0x020000,      /**< buffer */
  Pipe_Stream_f = 0x040000,          /**< FIFO buffer */
  Popen_Stream_f = 0x080000,         /**< popen open, pipes mosylyn */
  User_Stream_f = 0x100000,          /**< usually user_ipiy  */
  HAS_BOM_f = 0x200000,              /**< media for streamhas a BOM mar. */
  RepError_Prolog_f =
      0x400000,              /**< handle representation error as Prolog terms */
  RepError_Xml_f = 0x800000, /**< handle representation error as XML objects */
  DoNotCloseOnAbort_Stream_f =
      0x1000000, /**< do not close the stream after an abort event */
  Readline_Stream_f = 0x2000000, /**< the stream is a readline stream */
  FreeOnClose_Stream_f =
      0x4000000 /**< the stream buffer should be releaed on close */
} estream_f;

typedef uint64_t stream_flags_t;

/********* YAP C-Flags ***********************/

typedef enum {
  YAPC_ENABLE_GC, /* enable or disable garbage collection */
  YAPC_ENABLE_AGC /* enable or disable atom garbage collection */
} yap_flag_gc_t;

typedef enum yap_enum_reset_t {
  YAP_EXEC_ABSMI = 0,
  YAP_FULL_RESET = 1,
  YAP_RESET_FROM_RESTORE = 3
} yap_reset_t;

#endif /* _YAPDEFS_H */
