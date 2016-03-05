/*************************************************************************
*									 *
*	 YAP Prolog   %W% %G%
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		YAtom.h.m4						 *
* Last rev:	19/2/88							 *
* mods:									 *
* comments:	atom properties header file for YAP			 *
*									 *
*************************************************************************/

/* This code can only be defined *after* including Regs.h!!! */

#ifndef YATOM_H
#define YATOM_H 1

#ifdef USE_OFFSETS

INLINE_ONLY inline EXTERN Atom AbsAtom(AtomEntry *p);

INLINE_ONLY inline EXTERN Atom AbsAtom(AtomEntry *p) {
  return (Atom)(Addr(p) - AtomBase);
}

INLINE_ONLY inline EXTERN AtomEntry *RepAtom(Atom a);

INLINE_ONLY inline EXTERN AtomEntry *RepAtom(Atom a) {
  return (AtomEntry *) (AtomBase + Unsigned (a);
}

#else

INLINE_ONLY inline EXTERN Atom AbsAtom(AtomEntry *p);















  INLINE_ONLY inline EXTERN Atom AbsAtom(AtomEntry *p) { return (Atom)(p); }

INLINE_ONLY inline EXTERN AtomEntry *RepAtom(Atom a);

INLINE_ONLY inline EXTERN AtomEntry *RepAtom(Atom a) {
  return (AtomEntry *)(a);
}

#endif

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN Prop AbsProp(PropEntry *p);

INLINE_ONLY inline EXTERN Prop AbsProp(PropEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

INLINE_ONLY inline EXTERN PropEntry *RepProp(Prop p);

INLINE_ONLY inline EXTERN PropEntry *RepProp(Prop p) {
  return (PropEntry *)(AtomBase + Unsigned(p));
}

#else

INLINE_ONLY inline EXTERN Prop AbsProp(PropEntry *p);

INLINE_ONLY inline EXTERN Prop AbsProp(PropEntry *p) { return (Prop)(p); }

INLINE_ONLY inline EXTERN PropEntry *RepProp(Prop p);

INLINE_ONLY inline EXTERN PropEntry *RepProp(Prop p) {
  return (PropEntry *)(p);
}

#endif

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN FunctorEntry *RepFunctorProp(Prop p);

INLINE_ONLY inline EXTERN FunctorEntry *RepFunctorProp(Prop p) {
  return (FunctorEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsFunctorProp(FunctorEntry *p);

INLINE_ONLY inline EXTERN Prop AbsFunctorProp(FunctorEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN FunctorEntry *RepFunctorProp(Prop p);

INLINE_ONLY inline EXTERN FunctorEntry *RepFunctorProp(Prop p) {
  return (FunctorEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsFunctorProp(FunctorEntry *p);

INLINE_ONLY inline EXTERN Prop AbsFunctorProp(FunctorEntry *p) {
  return (Prop)(p);
}

#endif

INLINE_ONLY inline EXTERN Int ArityOfFunctor(Functor);

INLINE_ONLY inline EXTERN Int ArityOfFunctor(Functor Fun) {
  return (Int)(((FunctorEntry *)Fun)->ArityOfFE);
}

INLINE_ONLY inline EXTERN Atom NameOfFunctor(Functor);

INLINE_ONLY inline EXTERN Atom NameOfFunctor(Functor Fun) {
  return (Atom)(((FunctorEntry *)Fun)->NameOfFE);
}

INLINE_ONLY inline EXTERN PropFlags IsFunctorProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsFunctorProperty(int flags) {
  return (PropFlags)((flags == FunctorProperty));
}

/* summary of property codes used

        00 00	predicate entry
        80 00	db property
        bb 00	functor entry
        ff df	sparse functor
        ff ex	arithmetic property
        ff f4   translation
        ff f5   blob
        ff f6   hold
        ff f7   array
        ff f8   wide atom
        ff fa   module property
        ff fb   blackboard property
        ff fc	value property
        ff fd	global property
        ff fe	flag property
        ff ff	op property
*/

/*	Global Variable property */
typedef struct global_entry {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t GRWLock; /* a simple lock to protect this entry */
#if THREADS
  unsigned int owner_id; /* owner thread */
#endif
#endif
  struct AtomEntryStruct *AtomOfGE; /* parent atom for deletion */
  struct global_entry *NextGE;      /* linked list of global entries */
  Term global;                      /* index in module table                */
  Term AttChain;                    /* index in module table                */
} GlobalEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN GlobalEntry *RepGlobalProp(Prop p);

INLINE_ONLY inline EXTERN GlobalEntry *RepGlobalProp(Prop p) {
  return (GlobalEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsGlobalProp(GlobalEntry *p);

INLINE_ONLY inline EXTERN Prop AbsGlobalProp(GlobalEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN GlobalEntry *RepGlobalProp(Prop p);

INLINE_ONLY inline EXTERN GlobalEntry *RepGlobalProp(Prop p) {
  return (GlobalEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsGlobalProp(GlobalEntry *p);

INLINE_ONLY inline EXTERN Prop AbsGlobalProp(GlobalEntry *p) {
  return (Prop)(p);
}

#endif

#define GlobalProperty ((PropFlags)0xfffd)

INLINE_ONLY inline EXTERN PropFlags IsGlobalProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsGlobalProperty(int flags) {
  return (PropFlags)((flags == GlobalProperty));
}

/*	Wide Atom property 						*/
typedef struct {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  UInt SizeOfAtom;    /* index in module table                */
} WideAtomEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN WideAtomEntry *RepWideAtomProp(Prop p);

INLINE_ONLY inline EXTERN WideAtomEntry *RepWideAtomProp(Prop p) {
  return (WideAtomEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsWideAtomProp(WideAtomEntry *p);

INLINE_ONLY inline EXTERN Prop AbsWideAtomProp(WideAtomEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN WideAtomEntry *RepWideAtomProp(Prop p);

INLINE_ONLY inline EXTERN WideAtomEntry *RepWideAtomProp(Prop p) {
  return (WideAtomEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsWideAtomProp(WideAtomEntry *p);

INLINE_ONLY inline EXTERN Prop AbsWideAtomProp(WideAtomEntry *p) {
  return (Prop)(p);
}

#endif

#define WideAtomProperty ((PropFlags)0xfff8)

INLINE_ONLY inline EXTERN PropFlags IsWideAtomProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsWideAtomProperty(int flags) {
  return (PropFlags)((flags == WideAtomProperty));
}

INLINE_ONLY inline EXTERN int IsWideAtom(Atom);

INLINE_ONLY inline EXTERN int IsWideAtom(Atom at) {
  return RepAtom(at)->PropsOfAE &&
         IsWideAtomProperty(RepWideAtomProp(RepAtom(at)->PropsOfAE)->KindOfPE);
}

/*	Module property 						*/
typedef struct mod_entry {
  Prop NextOfPE;                /* used to chain properties            */
  PropFlags KindOfPE;           /* kind of property                    */
  struct pred_entry *PredForME; /* index in module table               */
  Atom AtomOfME;                /* module's name	                */
  Atom OwnerFile;               /* module's owner file	                */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t ModRWLock; /* a read-write lock to protect the entry */
#endif
  unsigned int flags;       /* Module local flags (from SWI compat) */
  struct mod_entry *NextME; /* next module                         */
} ModEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN ModEntry *RepModProp(Prop p);

INLINE_ONLY inline EXTERN ModEntry *RepModProp(Prop p) {
  return (ModEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsModProp(ModEntry *p);

INLINE_ONLY inline EXTERN Prop AbsModProp(ModEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN ModEntry *RepModProp(Prop p);

INLINE_ONLY inline EXTERN ModEntry *RepModProp(Prop p) {
  return (ModEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsModProp(ModEntry *p);

INLINE_ONLY inline EXTERN Prop AbsModProp(ModEntry *p) { return (Prop)(p); }

#define ModToTerm(m) (m == PROLOG_MODULE ? TermProlog : m)

#endif

#define ModProperty ((PropFlags)0xfffa)

INLINE_ONLY inline EXTERN PropFlags IsModProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsModProperty(int flags) {
  return (PropFlags)((flags == ModProperty));
}

/* Flags on module.  Most of these flags are copied to the read context
   in pl-read.c.
*/
#define M_SYSTEM (0x0001)     /* system module */
#define M_CHARESCAPE (0x0002) /* module */
#define DBLQ_CHARS (0x0004)   /* "ab" --> ['a', 'b'] */
#define DBLQ_ATOM (0x0008)    /* "ab" --> 'ab' */
#define DBLQ_STRING (0x0010)  /* "ab" --> "ab" */
#define DBLQ_CODES (0x0020)   /* "ab" --> [0'a, 0'b] */
#define DBLQ_MASK (DBLQ_CHARS | DBLQ_ATOM | DBLQ_STRING | DBLQ_CODES)
#define BCKQ_CHARS (0x0040)  /* `ab` --> ['a', 'b'] */
#define BCKQ_ATOM (0x0080)   /* `ab` --> 'ab' */
#define BCKQ_STRING (0x0100) /* `ab` --> "ab" */
#define BCKQ_CODES (0x0200)  /* `ab` --> [0'a, 0'b] */
#define BCKQ_MASK (BCKQ_CHARS | BCKQ_ATOM | BCKQ_STRING | BCKQ_CODES)
#define UNKNOWN_FAIL (0x0400)      /* module */
#define UNKNOWN_WARNING (0x0800)   /* module */
#define UNKNOWN_ERROR (0x1000)     /* module */
#define UNKNOWN_FAST_FAIL (0x2000) /* module */
#define UNKNOWN_ABORT (0x4000)     /* module */
#define UNKNOWN_HALT (0x8000)      /* module */
#define UNKNOWN_MASK                                                           \
  (UNKNOWN_ERROR | UNKNOWN_WARNING | UNKNOWN_FAIL | UNKNOWN_FAST_FAIL |        \
   UNKNOWN_ABORT | UNKNOWN_HALT)

Term Yap_getUnknownModule(ModEntry *m);
void Yap_setModuleFlags(ModEntry *n, ModEntry *o);

/*	    operator property entry structure				*/
typedef struct operator_entry {
  Prop NextOfPE;      /* used to chain properties     */
  PropFlags KindOfPE; /* kind of property             */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t OpRWLock; /* a read-write lock to protect the entry */
#endif
  Atom OpName;                   /* atom name		        */
  Term OpModule;                 /* module of predicate          */
  struct operator_entry *OpNext; /* next in list of operators  */
  BITS16 Prefix, Infix, Posfix;  /* precedences                  */
} OpEntry;
#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN OpEntry *RepOpProp(Prop p);

INLINE_ONLY inline EXTERN OpEntry *RepOpProp(Prop p) {
  return (OpEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsOpProp(OpEntry *p);

INLINE_ONLY inline EXTERN Prop AbsOpProp(OpEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN OpEntry *RepOpProp(Prop p);

INLINE_ONLY inline EXTERN OpEntry *RepOpProp(Prop p) { return (OpEntry *)(p); }

INLINE_ONLY inline EXTERN Prop AbsOpProp(OpEntry *p);

INLINE_ONLY inline EXTERN Prop AbsOpProp(OpEntry *p) { return (Prop)(p); }

#endif
#define OpProperty ((PropFlags)0xffff)

INLINE_ONLY inline EXTERN PropFlags IsOpProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsOpProperty(int flags) {
  return (PropFlags)((flags == OpProperty));
}

typedef enum { INFIX_OP = 0, POSFIX_OP = 1, PREFIX_OP = 2 } op_type;

OpEntry *Yap_GetOpProp(Atom, op_type, Term CACHE_TYPE);

int Yap_IsPrefixOp(Atom, int *, int *);
int Yap_IsOp(Atom);
int Yap_IsInfixOp(Atom, int *, int *, int *);
int Yap_IsPosfixOp(Atom, int *, int *);

/* defines related to operator specifications				*/
#define MaskPrio 0x0fff
#define DcrlpFlag 0x1000
#define DcrrpFlag 0x2000

typedef union arith_ret *eval_ret;

/*	    expression property	entry structure			*/
typedef struct {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  unsigned int ArityOfEE;
  BITS16 ENoOfEE;
  BITS16 FlagsOfEE;
  /* operations that implement the expression */
  int FOfEE;
} ExpEntry;
#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN ExpEntry *RepExpProp(Prop p);

INLINE_ONLY inline EXTERN ExpEntry *RepExpProp(Prop p) {
  return (ExpEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsExpProp(ExpEntry *p);

INLINE_ONLY inline EXTERN Prop AbsExpProp(ExpEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN ExpEntry *RepExpProp(Prop p);

INLINE_ONLY inline EXTERN ExpEntry *RepExpProp(Prop p) {
  return (ExpEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsExpProp(ExpEntry *p);

INLINE_ONLY inline EXTERN Prop AbsExpProp(ExpEntry *p) { return (Prop)(p); }

#endif
#define ExpProperty 0xffe0

/* only unary and binary expressions are acceptable */

INLINE_ONLY inline EXTERN PropFlags IsExpProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsExpProperty(int flags) {
  return (PropFlags)((flags == ExpProperty));
}

/*		value property entry structure				*/
typedef struct {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t VRWLock; /* a read-write lock to protect the entry */
#endif
  Term ValueOfVE; /* (atomic) value associated with the atom */
} ValEntry;
#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN ValEntry *RepValProp(Prop p);

INLINE_ONLY inline EXTERN ValEntry *RepValProp(Prop p) {
  return (ValEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsValProp(ValEntry *p);

INLINE_ONLY inline EXTERN Prop AbsValProp(ValEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN ValEntry *RepValProp(Prop p);

INLINE_ONLY inline EXTERN ValEntry *RepValProp(Prop p) {
  return (ValEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsValProp(ValEntry *p);

INLINE_ONLY inline EXTERN Prop AbsValProp(ValEntry *p) { return (Prop)(p); }

#endif
#define ValProperty ((PropFlags)0xfffc)

/*	    predicate property entry structure				*/
/*  AsmPreds are things like var, nonvar, atom ...which are implemented
            through dedicated machine instructions. In this case the 8 lower
            bits of PredFlags are used to hold the machine instruction code
            for	the pred.
    C_Preds are	things write, read, ...	implemented in C. In this case
            CodeOfPred holds the address of the	correspondent C-function.

don't forget to also add in qly.h
*/
typedef uint64_t pred_flags_t;

#define DiscontiguousPredFlag                                                  \
  ((                                                                           \
      pred_flags_t)0x1000000000) /* predicates whose clauses may be all-over   \
                                    the place.. */
#define SysExportPredFlag ((pred_flags_t)0x800000000)
/* reuse export list to prolog module. */
#define NoTracePredFlag                                                        \
  ((pred_flags_t)0x400000000) /* cannot trace this predicate */
#define NoSpyPredFlag                                                          \
  ((pred_flags_t)0x200000000) /* cannot spy this predicate */
#define QuasiQuotationPredFlag                                                 \
  ((pred_flags_t)0x100000000) /* SWI-like quasi quotations */
#define MegaClausePredFlag                                                     \
  ((pred_flags_t)0x80000000) /* predicate is implemented as a mega-clause */
#define ThreadLocalPredFlag ((pred_flags_t)0x40000000) /* local to a thread */
#define MultiFileFlag ((pred_flags_t)0x20000000)       /* is multi-file */
#define UserCPredFlag ((pred_flags_t)0x10000000) /* CPred defined by the user  \
                                                    */
#define LogUpdatePredFlag                                                      \
  ((pred_flags_t)0x08000000) /* dynamic predicate with log. upd. sem. */
#define InUsePredFlag ((pred_flags_t)0x04000000)  /* count calls to pred */
#define CountPredFlag ((pred_flags_t)0x02000000)  /* count calls to pred */
#define HiddenPredFlag ((pred_flags_t)0x01000000) /* invisible predicate */
#define CArgsPredFlag                                                          \
  ((pred_flags_t)0x00800000) /* SWI-like C-interface pred. */
#define SourcePredFlag                                                         \
  ((pred_flags_t)0x00400000) /* static predicate with source declaration */
#define MetaPredFlag                                                           \
  ((pred_flags_t)0x00200000) /* predicate subject to a meta declaration */
#define SyncPredFlag                                                           \
  ((pred_flags_t)0x00100000) /* has to synch before it can execute */
#define NumberDBPredFlag ((pred_flags_t)0x00080000) /* entry for an atom key   \
                                                       */
#define AtomDBPredFlag ((pred_flags_t)0x00040000)   /* entry for a number key */
// #define GoalExPredFlag  ((pred_flags_t)0x00020000)	/// predicate that is
// called by goal_expand */
#define TestPredFlag ((pred_flags_t)0x00010000) /* is a test (optim. comit) */
#define AsmPredFlag ((pred_flags_t)0x00008000)  /* inline */
#define StandardPredFlag ((pred_flags_t)0x00004000) /* system predicate */
#define DynamicPredFlag ((pred_flags_t)0x00002000)  /* dynamic predicate */
#define CPredFlag ((pred_flags_t)0x00001000)        /* written in C */
#define SafePredFlag ((pred_flags_t)0x00000800) /* does not alter arguments */
#define CompiledPredFlag ((pred_flags_t)0x00000400) /* is static */
#define IndexedPredFlag ((pred_flags_t)0x00000200)  /* has indexing code */
#define SpiedPredFlag ((pred_flags_t)0x00000100)    /* is a spy point */
#define BinaryPredFlag ((pred_flags_t)0x00000080)   /* test predicate */
#define TabledPredFlag ((pred_flags_t)0x00000040)   /* is tabled */
#define SequentialPredFlag                                                     \
  ((pred_flags_t)0x00000020) /* may not create parallel choice points! */
#define ProfiledPredFlag                                                       \
  ((pred_flags_t)0x00000010)                     /* pred is being profiled   */
#define BackCPredFlag ((pred_flags_t)0x00000008) /*	Myddas Imported pred       \
                                                    */
#define ModuleTransparentPredFlag                                              \
  ((pred_flags_t)0x00000004)                      /* ModuleTransparent pred  */
#define SWIEnvPredFlag ((pred_flags_t)0x00000002) /* new SWI interface */
#define UDIPredFlag ((pred_flags_t)0x00000001)    /* User Defined Indexing */

#define SystemPredFlags                                                        \
  (AsmPredFlag | StandardPredFlag | CPredFlag | BinaryPredFlag | BackCPredFlag)
#define ForeignPredFlags                                                       \
  (AsmPredFlag | SWIEnvPredFlag | CPredFlag | BinaryPredFlag | UDIPredFlag  | CArgsPredFlag | UserCPredFlag|SafePredFlag|BackCPredFlag)

#define StatePredFlags (InUsePredFlag|CountPredFlag|SpiedPredFlag|IndexedPredFlag )
#define is_system(pe) (pe->PredFlags & SystemPredFlags)
#define is_dynamic(pe) (pe->PredFlags & DynamicPredFlag)
#define is_foreign(pe) (pe->PredFlags & ForeignPredFlags)
#define is_static(pe) (pe->PredFlags & CompiledPredFlag)
#define is_logupd(pe) (pe->PredFlags & LogUpdatePredFlag)
#ifdef TABLING
#define is_tabled(pe) (pe->PredFlags & TabledPredFlag)
#endif /* TABLING */

/* profile data */
typedef struct {
  UInt NOfEntries;       /* nbr of times head unification succeeded */
  UInt NOfHeadSuccesses; /* nbr of times head unification succeeded */
  UInt NOfRetries;       /* nbr of times a clause for the pred
                                    was retried */
#if defined(YAPOR) || defined(THREADS)
  lockvar lock; /* a simple lock to protect this entry */
#endif
} profile_data;

typedef enum { LUCALL_EXEC, LUCALL_ASSERT, LUCALL_RETRACT } timestamp_type;

#define TIMESTAMP_EOT ((UInt)(~0L))
#define TIMESTAMP_RESET (TIMESTAMP_EOT - 1024)

typedef struct pred_entry {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  struct yami *CodeOfPred;
  OPCODE OpcodeOfPred; /* undefcode, indexcode, spycode, ....  */
  pred_flags_t PredFlags;
  UInt ArityOfPE; /* arity of property                    */
  union {
    struct {
      struct yami *TrueCodeOfPred; /* code address                         */
      struct yami *FirstClause;
      struct yami *LastClause;
      UInt NOfClauses;
      OPCODE ExpandCode;
    } p_code;
    CPredicate f_code;
    CmpPredicate d_code;
  } cs;                  /* if needing to spy or to lock         */
  Functor FunctorOfPred; /* functor for Predicate                */
  union {
    Atom OwnerFile; /* File where the predicate was defined */
    Int IndxId;     /* Index for a certain key */
  } src;
#if defined(YAPOR) || defined(THREADS)
  lockvar PELock; /* a simple lock to protect expansion */
#endif
#ifdef TABLING
  tab_ent_ptr TableOfPred;
#endif /* TABLING */
#ifdef BEAM
  struct Predicates *beamTable;
#endif
  Term ModuleOfPred; /* module for this definition           */
  UInt TimeStampOfPred;
  timestamp_type LastCallOfPred;
  /* This must be at an odd number of cells, otherwise it
     will not be aligned on RISC machines */
  profile_data *StatisticsForPred;     /* enable profiling for predicate  */
  struct pred_entry *NextPredOfModule; /* next pred for same module   */
  struct pred_entry *NextPredOfHash;   /* next pred for same module   */
} PredEntry;
#define PEProp ((PropFlags)(0x0000))

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN PredEntry *RepPredProp(Prop p);

INLINE_ONLY inline EXTERN PredEntry *RepPredProp(Prop p) {
  return (PredEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsPredProp(PredEntry *p);

INLINE_ONLY inline EXTERN Prop AbsPredProp(PredEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN PredEntry *RepPredProp(Prop p);

INLINE_ONLY inline EXTERN PredEntry *RepPredProp(Prop p) {

  return (PredEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsPredProp(PredEntry *p);

INLINE_ONLY inline EXTERN Prop AbsPredProp(PredEntry *p) { return (Prop)(p); }

#endif

INLINE_ONLY inline EXTERN PropFlags IsPredProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsPredProperty(int flags) {
  return (PropFlags)((flags == PEProp));
}

/* Flags for code or dbase entry */
/* There are several flags for code and data base entries */
typedef enum {
  ExoMask = 0x1000000,       /* is  exo code */
  FuncSwitchMask = 0x800000, /* is a switch of functors */
  HasDBTMask = 0x400000,     /* includes a pointer to a DBTerm */
  MegaMask = 0x200000,       /* mega clause */
  FactMask = 0x100000,       /* a fact */
  SwitchRootMask = 0x80000,  /* root for the index tree */
  SwitchTableMask = 0x40000, /* switch table */
  HasBlobsMask = 0x20000,    /* blobs which may be in use */
  ProfFoundMask = 0x10000,   /* clause is being counted by profiler */
  DynamicMask = 0x8000,      /* dynamic predicate */
  InUseMask = 0x4000,        /* this block is being used */
  ErasedMask = 0x2000,       /* this block has been erased */
  IndexMask = 0x1000,        /* indexing code */
  DBClMask = 0x0800,         /* data base structure */
  LogUpdRuleMask = 0x0400,   /* code is for a log upd rule with env */
  LogUpdMask = 0x0200,       /* logic update index. */
  StaticMask = 0x0100,       /* static predicates */
  DirtyMask = 0x0080,        /* LUIndices  */
  HasCutMask = 0x0040,       /* ! */
  SrcMask = 0x0020,          /* has a source term, only for static references */
  /* other flags belong to DB */
} dbentry_flags;

/* predicate initialization */
void Yap_InitCPred(const char *, UInt, CPredicate, pred_flags_t);
void Yap_InitAsmPred(const char *, UInt, int, CPredicate, pred_flags_t);
void Yap_InitCmpPred(const char *, UInt, CmpPredicate, pred_flags_t);
void Yap_InitCPredBack(const char *, UInt, unsigned int, CPredicate, CPredicate,
                       pred_flags_t);
void Yap_InitCPredBackCut(const char *, UInt, unsigned int, CPredicate,
                          CPredicate, CPredicate, pred_flags_t);
void Yap_InitCPredBack_(const char *, UInt, unsigned int, CPredicate,
                        CPredicate, CPredicate, pred_flags_t);

/* *********************** DBrefs **************************************/

typedef struct DB_TERM {
#ifdef COROUTINING
  union {
    CELL attachments; /* attached terms */
    Int line_number;
    struct DB_TERM *NextDBT;
  } ag;
#endif
  struct DB_STRUCT **DBRefs; /* pointer to other references     */
  CELL NOfCells;             /* Size of Term                         */
  CELL Entry;                /* entry point                          */
  Term Contents[MIN_ARRAY];  /* stored term                      */
} DBTerm;

INLINE_ONLY inline EXTERN DBTerm *TermToDBTerm(Term);

INLINE_ONLY inline EXTERN DBTerm *TermToDBTerm(Term X) {
  if (IsPairTerm(X)) {
    return (DBTerm *)((unsigned char *)RepPair(X) - (CELL) &
                      (((DBTerm *)NULL)->Contents));
  } else {
    return (DBTerm *)((unsigned char *)RepAppl(X) - (CELL) &
                      (((DBTerm *)NULL)->Contents));
  }
}

/* The ordering of the first 3 fields should be compatible with lu_clauses */
typedef struct DB_STRUCT {
  Functor id; /* allow pointers to this struct to id  */
  /*   as dbref                           */
  CELL Flags; /* Term Flags                           */
#if defined(YAPOR) || defined(THREADS)
  lockvar lock; /* a simple lock to protect this entry */
#endif
#if MULTIPLE_STACKS
  Int ref_count; /* how many branches are using this entry */
#endif
  CELL NOfRefsTo;                /* Number of references pointing here   */
  struct struct_dbentry *Parent; /* key of DBase reference               */
  struct yami *Code;             /* pointer to code if this is a clause  */
  struct DB_STRUCT *Prev;        /* Previous element in chain            */
  struct DB_STRUCT *Next;        /* Next element in chain                */
  struct DB_STRUCT *p, *n;       /* entry's age, negative if from recorda,
                                    positive if it was recordz  */
  CELL Mask;                     /* parts that should be cleared         */
  CELL Key;                      /* A mask that can be used to check before
                                    you unify */
  DBTerm DBT;
} DBStruct;

#define DBStructFlagsToDBStruct(X)                                             \
  ((DBRef)((unsigned char *)(X) - (CELL) & (((DBRef)NULL)->Flags)))

#if MULTIPLE_STACKS
#define INIT_DBREF_COUNT(X) (X)->ref_count = 0
#define INC_DBREF_COUNT(X) (X)->ref_count++
#define DEC_DBREF_COUNT(X) (X)->ref_count--
#define DBREF_IN_USE(X) ((X)->ref_count != 0)
#else
#define INIT_DBREF_COUNT(X)
#define INC_DBREF_COUNT(X)
#define DEC_DBREF_COUNT(X)
#define DBREF_IN_USE(X) ((X)->Flags & InUseMask)
#endif

typedef DBStruct *DBRef;

/* extern Functor FunctorDBRef; */

INLINE_ONLY inline EXTERN int IsDBRefTerm(Term);

INLINE_ONLY inline EXTERN int IsDBRefTerm(Term t) {
  return (int)(IsApplTerm(t) && FunctorOfTerm(t) == FunctorDBRef);
}

INLINE_ONLY inline EXTERN Term MkDBRefTerm(DBRef);

INLINE_ONLY inline EXTERN Term MkDBRefTerm(DBRef p) {
  return (Term)((AbsAppl(((CELL *)(p)))));
}

INLINE_ONLY inline EXTERN DBRef DBRefOfTerm(Term t);

INLINE_ONLY inline EXTERN DBRef DBRefOfTerm(Term t) {
  return (DBRef)(((DBRef)(RepAppl(t))));
}

INLINE_ONLY inline EXTERN int IsRefTerm(Term);

INLINE_ONLY inline EXTERN int IsRefTerm(Term t) {
  return (int)(IsApplTerm(t) && FunctorOfTerm(t) == FunctorDBRef);
}

INLINE_ONLY inline EXTERN CODEADDR RefOfTerm(Term t);

INLINE_ONLY inline EXTERN CODEADDR RefOfTerm(Term t) {
  return (CODEADDR)(DBRefOfTerm(t));
}

typedef struct struct_dbentry {
  Prop NextOfPE;          /* used to chain properties             */
  PropFlags KindOfPE;     /* kind of property                     */
  unsigned int ArityOfDB; /* kind of property                     */
  Functor FunctorOfDB;    /* functor for this property            */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t DBRWLock; /* a simple lock to protect this entry */
#endif
  DBRef First;     /* first DBase entry                    */
  DBRef Last;      /* last DBase entry                     */
  Term ModuleOfDB; /* module for this definition           */
  DBRef F0, L0;    /* everyone                          */
} DBEntry;
typedef DBEntry *DBProp;
#define DBProperty ((PropFlags)0x8000)

typedef struct {
  Prop NextOfPE;          /* used to chain properties             */
  PropFlags KindOfPE;     /* kind of property                     */
  unsigned int ArityOfDB; /* kind of property                     */
  Functor FunctorOfDB;    /* functor for this property            */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t DBRWLock; /* a simple lock to protect this entry */
#endif
  DBRef First;     /* first DBase entry                    */
  DBRef Last;      /* last DBase entry                     */
  Term ModuleOfDB; /* module for this definition           */
  Int NOfEntries;  /* age counter                          */
  DBRef Index;     /* age counter                          */
} LogUpdDBEntry;
typedef LogUpdDBEntry *LogUpdDBProp;
#define CodeDBBit 0x2

#define CodeDBProperty (DBProperty | CodeDBBit)

INLINE_ONLY inline EXTERN PropFlags IsDBProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsDBProperty(int flags) {
  return (PropFlags)((flags & ~CodeDBBit) == DBProperty);
}

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN DBProp RepDBProp(Prop p);

INLINE_ONLY inline EXTERN DBProp RepDBProp(Prop p) {
  return (DBProp)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsDBProp(DBProp p);

INLINE_ONLY inline EXTERN Prop AbsDBProp(DBProp p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN DBProp RepDBProp(Prop p);

INLINE_ONLY inline EXTERN DBProp RepDBProp(Prop p) { return (DBProp)(p); }

INLINE_ONLY inline EXTERN Prop AbsDBProp(DBProp p);

INLINE_ONLY inline EXTERN Prop AbsDBProp(DBProp p) { return (Prop)(p); }

#endif

/* These are the actual flags for DataBase terms */
typedef enum {
  DBAtomic = 0x1,
  DBVar = 0x2,
  DBNoVars = 0x4,
  DBComplex = 0x8,
  DBCode = 0x10,
  DBNoCode = 0x20,
  DBWithRefs = 0x40
} db_term_flags;

typedef struct {
  Prop NextOfPE;      /* used to chain properties                */
  PropFlags KindOfPE; /* kind of property                        */
  Atom KeyOfBB;       /* functor for this property               */
  Term Element;       /* blackboard element                      */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t BBRWLock; /* a read-write lock to protect the entry */
#endif
  Term ModuleOfBB; /* module for this definition             */
} BlackBoardEntry;
typedef BlackBoardEntry *BBProp;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN BlackBoardEntry *RepBBProp(Prop p);

INLINE_ONLY inline EXTERN BlackBoardEntry *RepBBProp(Prop p) {
  return (BlackBoardEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsBBProp(BlackBoardEntry *p);

INLINE_ONLY inline EXTERN Prop AbsBBProp(BlackBoardEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN BlackBoardEntry *RepBBProp(Prop p);

INLINE_ONLY inline EXTERN BlackBoardEntry *RepBBProp(Prop p) {
  return (BlackBoardEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsBBProp(BlackBoardEntry *p);

INLINE_ONLY inline EXTERN Prop AbsBBProp(BlackBoardEntry *p) {
  return (Prop)(p);
}

#endif

#define BBProperty ((PropFlags)0xfffb)

INLINE_ONLY inline EXTERN PropFlags IsBBProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsBBProperty(int flags) {
  return (PropFlags)((flags == BBProperty));
}

/*		hold property entry structure				*/
typedef struct hold_entry {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  UInt RefsOfPE;      /* used to count the number of holds    */
} HoldEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN HoldEntry *RepHoldProp(Prop p);

INLINE_ONLY inline EXTERN HoldEntry *RepHoldProp(Prop p) {
  return (HoldEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsHoldProp(HoldEntry *p);

INLINE_ONLY inline EXTERN Prop AbsHoldProp(HoldEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN HoldEntry *RepHoldProp(Prop p);

INLINE_ONLY inline EXTERN HoldEntry *RepHoldProp(Prop p) {
  return (HoldEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsHoldProp(HoldEntry *p);

INLINE_ONLY inline EXTERN Prop AbsHoldProp(HoldEntry *p) { return (Prop)(p); }

#endif

#define HoldProperty 0xfff6

/*		translation property entry structure */
typedef struct translation_entry {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  arity_t arity;      /* refers to atom (0) or functor(N > 0) */
  Int Translation;    /* used to hash the atom as an integer; */
} TranslationEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN TranslationEntry *RepTranslationProp(Prop p);

INLINE_ONLY inline EXTERN TranslationEntry *RepTranslationProp(Prop p) {
  return (TranslationEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsTranslationProp(TranslationEntry *p);

INLINE_ONLY inline EXTERN Prop AbsTranslationProp(TranslationEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN TranslationEntry *RepTranslationProp(Prop p);

INLINE_ONLY inline EXTERN TranslationEntry *RepTranslationProp(Prop p) {
  return (TranslationEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsTranslationProp(TranslationEntry *p);

INLINE_ONLY inline EXTERN Prop AbsTranslationProp(TranslationEntry *p) {
  return (Prop)(p);
}

#endif
#define TranslationProperty 0xfff4

bool Yap_PutAtomTranslation(Atom a, arity_t arity, Int i);

/* get translation prop for atom;               */
static inline TranslationEntry *Yap_GetTranslationProp(Atom at, arity_t arity) {
  Prop p0;
  AtomEntry *ae = RepAtom(at);
  TranslationEntry *p;

  READ_LOCK(ae->ARWLock);
  p = RepTranslationProp(p0 = ae->PropsOfAE);
  while (p0 && (p->KindOfPE != TranslationProperty || p->arity != arity))
    p = RepTranslationProp(p0 = p->NextOfPE);
  READ_UNLOCK(ae->ARWLock);
  if (p0 == NIL)
    return (TranslationEntry *)NULL;
  p->arity = arity;
  return p;
}

INLINE_ONLY inline EXTERN PropFlags IsTranslationProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsTranslationProperty(int flags) {
  return (PropFlags)((flags == TranslationProperty));
}

/*** handle named mutexes */

/*              named mutex property entry structure */
typedef struct mutex_entry {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  void *Mutex;        /* used to hash the atom as an integer; */
} MutexEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN MutexEntry *RepMutexProp(Prop p);

INLINE_ONLY inline EXTERN MutexEntry *RepMutexProp(Prop p) {
  return (MutexEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsMutexProp(MutexEntry *p);

INLINE_ONLY inline EXTERN Prop AbsMutexProp(MutexEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN MutexEntry *RepMutexProp(Prop p);

INLINE_ONLY inline EXTERN MutexEntry *RepMutexProp(Prop p) {
  return (MutexEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsMutexProp(MutexEntry *p);

INLINE_ONLY inline EXTERN Prop AbsMutexProp(MutexEntry *p) { return (Prop)(p); }

#endif
#define MutexProperty 0xfff5

bool Yap_PutAtomMutex(Atom a, void *ptr);

/* get mutex prop for atom;               */
static inline void *Yap_GetMutexFromProp(Atom at) {
  Prop p0;
  AtomEntry *ae = RepAtom(at);
  MutexEntry *p;

  READ_LOCK(ae->ARWLock);
  p = RepMutexProp(p0 = ae->PropsOfAE);
  while (p0 && p->KindOfPE != MutexProperty)
    p = RepMutexProp(p0 = p->NextOfPE);
  READ_UNLOCK(ae->ARWLock);
  if (p0 == NIL)
    return NULL;
  return p->Mutex;
}

INLINE_ONLY inline EXTERN PropFlags IsMutexProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsMutexProperty(int flags) {
  return (PropFlags)((flags == MutexProperty));
}

/* end of code for named mutexes */

typedef enum {
  STATIC_ARRAY = 1,
  DYNAMIC_ARRAY = 2,
  MMAP_ARRAY = 4,
  FIXED_ARRAY = 8
} array_type;

/*		array property entry structure				*/
/*		first case is for dynamic arrays */
typedef struct array_entry {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  Int ArrayEArity;    /* Arity of Array (positive)            */
  array_type TypeOfAE;
#if defined(YAPOR) || defined(THREADS)
  rwlock_t ArRWLock; /* a read-write lock to protect the entry */
#if THREADS
  unsigned int owner_id;
#endif
#endif
  struct array_entry *NextAE;
  Term ValueOfVE; /* Pointer to the actual array          */
} ArrayEntry;

/* second case is for static arrays */

typedef struct {
  Term tlive;
  Term tstore;
} live_term;

typedef union {
  Int *ints;
  char *chars;
  unsigned char *uchars;
  Float *floats;
  AtomEntry **ptrs;
  Term *atoms;
  Term *dbrefs;
  DBTerm **terms;
  live_term *lterms;
} statarray_elements;

/* next, the actual data structure */
typedef struct static_array_entry {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
  Int ArrayEArity;    /* Arity of Array (negative)            */
  array_type TypeOfAE;
#if defined(YAPOR) || defined(THREADS)
  rwlock_t ArRWLock; /* a read-write lock to protect the entry */
#endif
  struct static_array_entry *NextAE;
  static_array_types ArrayType; /* Type of Array Elements.              */
  statarray_elements ValueOfVE; /* Pointer to the Array itself  */
} StaticArrayEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN ArrayEntry *RepArrayProp(Prop p);

INLINE_ONLY inline EXTERN ArrayEntry *RepArrayProp(Prop p) {
  return (ArrayEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsArrayProp(ArrayEntry *p);

INLINE_ONLY inline EXTERN Prop AbsArrayProp(ArrayEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

INLINE_ONLY inline EXTERN StaticArrayEntry *RepStaticArrayProp(Prop p);

INLINE_ONLY inline EXTERN StaticArrayEntry *RepStaticArrayProp(Prop p) {
  return (StaticArrayEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsStaticArrayProp(StaticArrayEntry *p);

INLINE_ONLY inline EXTERN Prop AbsStaticArrayProp(StaticArrayEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN ArrayEntry *RepArrayProp(Prop p);

INLINE_ONLY inline EXTERN ArrayEntry *RepArrayProp(Prop p) {
  return (ArrayEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsArrayProp(ArrayEntry *p);

INLINE_ONLY inline EXTERN Prop AbsArrayProp(ArrayEntry *p) { return (Prop)(p); }

INLINE_ONLY inline EXTERN StaticArrayEntry *RepStaticArrayProp(Prop p);

INLINE_ONLY inline EXTERN StaticArrayEntry *RepStaticArrayProp(Prop p) {
  return (StaticArrayEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsStaticArrayProp(StaticArrayEntry *p);

INLINE_ONLY inline EXTERN Prop AbsStaticArrayProp(StaticArrayEntry *p) {
  return (Prop)(p);
}

#endif
#define ArrayProperty ((PropFlags)0xfff7)

INLINE_ONLY inline EXTERN int ArrayIsDynamic(ArrayEntry *);

INLINE_ONLY inline EXTERN int ArrayIsDynamic(ArrayEntry *are) {
  return (int)(((are)->TypeOfAE & DYNAMIC_ARRAY));
}

INLINE_ONLY inline EXTERN PropFlags IsArrayProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsArrayProperty(int flags) {
  return (PropFlags)((flags == ArrayProperty));
}

/*	SWI Blob property 						*/
typedef struct YAP_blob_prop_entry {
  Prop NextOfPE;                /* used to chain properties             */
  PropFlags KindOfPE;           /* kind of property                     */
  struct YAP_blob_t *blob_type; /* type of blob */
} YAP_BlobPropEntry;

#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN YAP_BlobPropEntry *RepBlobProp(Prop p);

INLINE_ONLY inline EXTERN YAP_BlobPropEntry *RepBlobProp(Prop p) {
  return (YAP_BlobPropEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN AtomEntry *AbsBlobProp(BlobPropEntry *p);

INLINE_ONLY inline EXTERN Prop AbsBlobProp(YAP_BlobPropEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN YAP_BlobPropEntry *RepBlobProp(Prop p);

INLINE_ONLY inline EXTERN YAP_BlobPropEntry *RepBlobProp(Prop p) {
  return (YAP_BlobPropEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsBlobProp(YAP_BlobPropEntry *p);

INLINE_ONLY inline EXTERN Prop AbsBlobProp(YAP_BlobPropEntry *p) {
  return (Prop)(p);
}

#endif

#define BlobProperty ((PropFlags)0xfffe)

INLINE_ONLY inline EXTERN PropFlags IsBlobProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsBlobProperty(int flags) {
  return (PropFlags)((flags == BlobProperty));
}

INLINE_ONLY inline EXTERN int IsBlob(Atom);

INLINE_ONLY inline EXTERN int IsBlob(Atom at) {
  return RepAtom(at)->PropsOfAE &&
         IsBlobProperty(RepBlobProp(RepAtom(at)->PropsOfAE)->KindOfPE);
}

INLINE_ONLY inline EXTERN PropFlags IsValProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsValProperty(int flags) {
  return (PropFlags)((flags == ValProperty));
}

/*		flag property entry structure				*/

typedef bool (*flag_func)(Term);

typedef struct {
  Prop NextOfPE;      /* used to chain properties             */
  PropFlags KindOfPE; /* kind of property                     */
#if defined(YAPOR) || defined(THREADS)
  rwlock_t VRWLock; /* a read-write lock to protect the entry */
#endif
  int FlagOfVE; /* (atomic) value associated with the atom */
  bool global, atomic, rw;
  flag_func type, helper;
} FlagEntry;
#if USE_OFFSETS_IN_PROPS

INLINE_ONLY inline EXTERN FlagEntry *RepFlagProp(Prop p);

INLINE_ONLY inline EXTERN FlagEntry *RepFlagProp(Prop p) {
  return (FlagEntry *)(AtomBase + Unsigned(p));
}

INLINE_ONLY inline EXTERN Prop AbsFlagProp(FlagEntry *p);

INLINE_ONLY inline EXTERN Prop AbsValProp(FlagEntry *p) {
  return (Prop)(Addr(p) - AtomBase);
}

#else

INLINE_ONLY inline EXTERN FlagEntry *RepFlagProp(Prop p);

INLINE_ONLY inline EXTERN FlagEntry *RepFlagProp(Prop p) {
  return (FlagEntry *)(p);
}

INLINE_ONLY inline EXTERN Prop AbsFlagProp(FlagEntry *p);

INLINE_ONLY inline EXTERN Prop AbsFlagProp(FlagEntry *p) { return (Prop)(p); }

#endif
#define FlagProperty ((PropFlags)0xfff9)

INLINE_ONLY inline EXTERN PropFlags IsFlagProperty(int);

INLINE_ONLY inline EXTERN PropFlags IsFlagProperty(int flags) {
  return (PropFlags)((flags == FlagProperty));
}

/* Proto types */

/* cdmgr.c */
int Yap_RemoveIndexation(PredEntry *);
void Yap_UpdateTimestamps(PredEntry *);

/* dbase.c */
void Yap_ErDBE(DBRef);
DBTerm *Yap_StoreTermInDB(Term, int);
DBTerm *Yap_StoreTermInDBPlusExtraSpace(Term, UInt, UInt *);
Term Yap_FetchTermFromDB(DBTerm *);
Term Yap_FetchClauseTermFromDB(DBTerm *);
Term Yap_PopTermFromDB(DBTerm *);
void Yap_ReleaseTermFromDB(DBTerm *);

/* init.c */
Atom Yap_GetOp(OpEntry *, int *, int);

/* vsc: redefined to GetAProp to avoid conflicts with Windows header files */
Prop Yap_GetAProp(Atom, PropFlags);
Prop Yap_GetAPropHavingLock(AtomEntry *, PropFlags);

#define PROLOG_MODULE 0

#include "YapHeap.h"

#define PredHashInitialSize ((UInt)1039)
#define PredHashIncrement ((UInt)7919)

/*************************************************************************************************
                                       flag support
*************************************************************************************************/

#include "YapFlags.h"
INLINE_ONLY EXTERN inline UInt PRED_HASH(FunctorEntry *, Term, UInt);

INLINE_ONLY EXTERN inline UInt PRED_HASH(FunctorEntry *fe, Term cur_mod,
                                         UInt size) {
  return (((CELL)fe + cur_mod) >> 2) % size;
}

INLINE_ONLY EXTERN inline Prop GetPredPropByFuncAndModHavingLock(FunctorEntry *,
                                                                 Term);
INLINE_ONLY EXTERN inline Prop PredPropByFuncAndMod(FunctorEntry *, Term);
INLINE_ONLY EXTERN inline Prop PredPropByAtomAndMod(Atom, Term);
INLINE_ONLY EXTERN inline Prop GetPredPropByFuncHavingLock(FunctorEntry *,
                                                           Term);
INLINE_ONLY EXTERN inline Prop PredPropByFunc(Functor fe, Term cur_mod);
INLINE_ONLY EXTERN inline Prop PredPropByAtom(Atom at, Term cur_mod);

#ifdef THREADS

Prop Yap_NewThreadPred(struct pred_entry *CACHE_TYPE);
Prop Yap_NewPredPropByFunctor(Functor, Term);
INLINE_ONLY EXTERN inline struct pred_entry *
Yap_GetThreadPred(struct pred_entry *CACHE_TYPE);

INLINE_ONLY EXTERN inline struct pred_entry *
Yap_GetThreadPred(struct pred_entry *ap USES_REGS) {
  Functor f = ap->FunctorOfPred;
  Term mod = ap->ModuleOfPred;
  Prop p0 = AbsPredProp(LOCAL_ThreadHandle.local_preds);

  while (p0) {
    PredEntry *ap = RepPredProp(p0);
    if (ap->FunctorOfPred == f && ap->ModuleOfPred == mod)
      return ap;
    p0 = ap->NextOfPE;
  }
  return RepPredProp(Yap_NewThreadPred(ap PASS_REGS));
}
#endif

INLINE_ONLY EXTERN inline Prop GetPredPropByFuncHavingLock(FunctorEntry *fe,
                                                           Term cur_mod) {
  PredEntry *p;

  if (!(p = RepPredProp(fe->PropsOfFE))) {
    return NIL;
  }
  if ((p->ModuleOfPred == cur_mod || !(p->ModuleOfPred))) {
#ifdef THREADS
    /* Thread Local Predicates */
    if (p->PredFlags & ThreadLocalPredFlag) {
      return AbsPredProp(Yap_GetThreadPred(p INIT_REGS));
    }
#endif
    return AbsPredProp(p);
  }
  if (p->NextOfPE) {
    UInt hash = PRED_HASH(fe, cur_mod, PredHashTableSize);
    READ_LOCK(PredHashRWLock);
    p = PredHash[hash];

    while (p) {
      if (p->FunctorOfPred == fe && p->ModuleOfPred == cur_mod) {
#ifdef THREADS
        /* Thread Local Predicates */
        if (p->PredFlags & ThreadLocalPredFlag) {
          READ_UNLOCK(PredHashRWLock);
          return AbsPredProp(Yap_GetThreadPred(p INIT_REGS));
        }
#endif
        READ_UNLOCK(PredHashRWLock);
        return AbsPredProp(p);
      }
      p = p->NextPredOfHash;
    }
    READ_UNLOCK(PredHashRWLock);
  }
  return NIL;
}

INLINE_ONLY EXTERN inline Prop PredPropByFunc(Functor fe, Term cur_mod)
/* get predicate entry for ap/arity; create it if neccessary.              */
{
  Prop p0;

  FUNC_WRITE_LOCK(fe);
  p0 = GetPredPropByFuncHavingLock(fe, cur_mod);
  if (p0) {
    FUNC_WRITE_UNLOCK(fe);
    return p0;
  }
  return Yap_NewPredPropByFunctor(fe, cur_mod);
}

INLINE_ONLY EXTERN inline Prop
GetPredPropByFuncAndModHavingLock(FunctorEntry *fe, Term cur_mod) {
  PredEntry *p;

  if (!(p = RepPredProp(fe->PropsOfFE))) {
    return NIL;
  }
  if (p->ModuleOfPred == cur_mod) {
#ifdef THREADS
    /* Thread Local Predicates */
    if (p->PredFlags & ThreadLocalPredFlag) {
      return AbsPredProp(Yap_GetThreadPred(p INIT_REGS));
    }
#endif
    return AbsPredProp(p);
  }
  if (p->NextOfPE) {
    UInt hash = PRED_HASH(fe, cur_mod, PredHashTableSize);
    READ_LOCK(PredHashRWLock);
    p = PredHash[hash];

    while (p) {
      if (p->FunctorOfPred == fe && p->ModuleOfPred == cur_mod) {
#ifdef THREADS
        /* Thread Local Predicates */
        if (p->PredFlags & ThreadLocalPredFlag) {
          READ_UNLOCK(PredHashRWLock);
          return AbsPredProp(Yap_GetThreadPred(p INIT_REGS));
        }
#endif
        READ_UNLOCK(PredHashRWLock);
        return AbsPredProp(p);
      }
      p = p->NextPredOfHash;
    }
    READ_UNLOCK(PredHashRWLock);
  }
  return NIL;
}

INLINE_ONLY EXTERN inline Prop PredPropByFuncAndMod(Functor fe, Term cur_mod)
/* get predicate entry for ap/arity; create it if neccessary.              */
{
  Prop p0;

  FUNC_WRITE_LOCK(fe);
  p0 = GetPredPropByFuncAndModHavingLock(fe, cur_mod);
  if (p0) {
    FUNC_WRITE_UNLOCK(fe);
    return p0;
  }
  return Yap_NewPredPropByFunctor(fe, cur_mod);
}

INLINE_ONLY EXTERN inline Prop PredPropByAtom(Atom at, Term cur_mod)
/* get predicate entry for ap/arity; create it if neccessary.              */
{
  Prop p0;
  AtomEntry *ae = RepAtom(at);

  WRITE_LOCK(ae->ARWLock);
  p0 = ae->PropsOfAE;
  while (p0) {
    PredEntry *pe = RepPredProp(p0);
    if (pe->KindOfPE == PEProp &&
        (pe->ModuleOfPred == cur_mod || !pe->ModuleOfPred)) {
#ifdef THREADS
      /* Thread Local Predicates */
      if (pe->PredFlags & ThreadLocalPredFlag) {
        WRITE_UNLOCK(ae->ARWLock);
        return AbsPredProp(Yap_GetThreadPred(pe INIT_REGS));
      }
#endif
      WRITE_UNLOCK(ae->ARWLock);
      return (p0);
    }
    p0 = pe->NextOfPE;
  }
  return Yap_NewPredPropByAtom(ae, cur_mod);
}

INLINE_ONLY EXTERN inline Prop PredPropByAtomAndMod(Atom at, Term cur_mod)
/* get predicate entry for ap/arity; create it if neccessary.              */
{
  Prop p0;
  AtomEntry *ae = RepAtom(at);

  WRITE_LOCK(ae->ARWLock);
  p0 = ae->PropsOfAE;
  while (p0) {
    PredEntry *pe = RepPredProp(p0);
    if (pe->KindOfPE == PEProp && (pe->ModuleOfPred == cur_mod)) {
#ifdef THREADS
      /* Thread Local Predicates */
      if (pe->PredFlags & ThreadLocalPredFlag) {
        WRITE_UNLOCK(ae->ARWLock);
        return AbsPredProp(Yap_GetThreadPred(pe INIT_REGS));
      }
#endif
      WRITE_UNLOCK(ae->ARWLock);
      return (p0);
    }
    p0 = pe->NextOfPE;
  }
  return Yap_NewPredPropByAtom(ae, cur_mod);
}

#if DEBUG_PELOCKING
#define PELOCK(I, Z)                                                           \
  {                                                                            \
    LOCK((Z)->PELock);                                                         \
    (Z)->StatisticsForPred->NOfEntries = (I);                                  \
    (Z)->StatisticsForPred->NOfHeadSuccesses = pthread_self();                 \
  }
#define UNLOCKPE(I, Z)                                                         \
  ((Z)->StatisticsForPred->NOfRetries = (I), UNLOCK((Z)->PELock))
#elif YAPOR || THREADS
#define PELOCK(I, Z) (LOCK((Z)->PELock))
#define UNLOCKPE(I, Z) (UNLOCK((Z)->PELock))
#else
#define PELOCK(I, Z)
#define UNLOCKPE(I, Z)
#endif

INLINE_ONLY EXTERN inline void AddPropToAtom(AtomEntry *, PropEntry *p);

INLINE_ONLY EXTERN inline void AddPropToAtom(AtomEntry *ae, PropEntry *p) {
  /* old properties should be always last, and wide atom properties
     should always be first */
  if (ae->PropsOfAE != NIL &&
      RepProp(ae->PropsOfAE)->KindOfPE == WideAtomProperty) {
    PropEntry *pp = RepProp(ae->PropsOfAE);
    p->NextOfPE = pp->NextOfPE;
    pp->NextOfPE = AbsProp(p);
  } else {
    p->NextOfPE = ae->PropsOfAE;
    ae->PropsOfAE = AbsProp(p);
  }
}

// auxiliary functions

INLINE_ONLY inline EXTERN const char *AtomName(Atom at);

/**
 * AtomName(Atom at): get a string with the name of an Atom. Assumes 8 bit
 *representation.
 *
 * @param at the atom
 *
 * @return a ponter to an immutable sequence of characters.
 */
INLINE_ONLY inline EXTERN const char *AtomName(Atom at) {
  return RepAtom(at)->rep.uStrOfAE;
}

INLINE_ONLY inline EXTERN const char *AtomTermName(Term t);

/**
 * AtomTermName(Term t): get a string with the name of a term storing an Atom. Assumes 8
 *bit representation.
 *
 * @param t the atom term
 *
 * @return a ponter to an immutable sequence of characters.
 *
 * @note: this routine does not support wide chars.
 */
INLINE_ONLY inline EXTERN const char *AtomTermName(Term t) {
  return RepAtom(AtomOfTerm(t))->rep.uStrOfAE;
}

#endif
