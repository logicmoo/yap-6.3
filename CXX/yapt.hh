
#ifndef YAPT_HH
#define YAPT_HH 1

extern "C" Term  YAP_ReadBuffer(const char *s, Term *tp);

/**
 * @brief Generic Prolog Term
 */
class YAPTerm {
  friend class YAPPredicate;
  friend class YAPPrologPredicate;
  friend class YAPQuery;
  friend class YAPModule;
  friend class YAPModuleProp;
  friend class YAPApplTerm;
protected:
  yhandle_t t; /// handle to term, equivalent to term_t
  void mk(Term t0); /// internal method to convert from term to handle
  Term gt(); /// get handle and obtain term
public:
  virtual ~YAPTerm() {};
  YAPTerm(Term tn) {  mk( tn ); } /// private method to convert from Term (internal YAP representation) to YAPTerm
  // do nothing constructor
  YAPTerm() { mk(TermNil); }
  /// integer to term
  YAPTerm(intptr_t i);
  /// pointer to term
  YAPTerm(void *ptr);
  /// parse string s and construct a term.
  YAPTerm(char *s) { Term tp ; mk( YAP_ReadBuffer(s,&tp) );  }
  /// extract the tag of a term, after dereferencing.
  YAP_tag_t  tag();
  /// copy the term ( term copy )
  YAPTerm  deepCopy();
  inline Term term() { return gt(); } /// from YAPTerm to Term (internal YAP representation)
  //const YAPTerm *vars();
  /// this term is == to t1
  bool exactlyEqual(YAPTerm t1);
  bool unify(YAPTerm t1);     /// t = t1
  bool unifiable(YAPTerm t1);  /// we can unify t and t1
  bool variant(YAPTerm t1);   /// t =@= t1, the two terms are equal up to variable renaming
  intptr_t hashTerm(size_t sz, size_t depth, bool variant); /// term hash,
  virtual bool isVar() { return IsVarTerm( gt() ); }   /// type check for unbound
  virtual bool isAtom() { return IsAtomTerm( gt() ); } ///  type check for atom
  virtual bool isInteger() { return IsIntegerTerm( gt() ); } /// type check for integer
  virtual bool isFloat() { return IsFloatTerm( gt() ); } /// type check for floating-point
  virtual bool isString() { return IsStringTerm( gt() ); } /// type check for a string " ... "
  virtual bool isCompound() { return !(IsVarTerm( gt() ) || IsNumTerm( gt() )); } /// is a primitive term
  virtual bool isAppl() { return IsApplTerm( gt() ); } /// is a structured term
  virtual bool isPair() { return IsPairTerm( gt() ); } /// is a pair term
  virtual bool isGround() { return Yap_IsGroundTerm( gt() ); } /// term is ground
  virtual bool isList() { return Yap_IsListTerm( gt() ); } /// term is a list

  /// extract the argument i of the term, where i in 1...arity
  inline YAPTerm getArg(int i) {
    BACKUP_MACHINE_REGS();
    Term t0 = gt();
    YAPTerm tf;
    if (IsApplTerm(t0))
      tf = YAPTerm(ArgOfTerm(i, t0));
    else if (IsPairTerm(t0)) {
      if (i==1)
        tf = YAPTerm(HeadOfTerm(t0));
      else if (i==2)
        tf =  YAPTerm(TailOfTerm(t0));
    } else {
      tf = YAPTerm((Term)0);
    }
    RECOVER_MACHINE_REGS();
    REGS_LOG( "after getArg H= %p, i=%d", HR, tf.gt()) ;
    return tf;
  }

  /// return a string with a textual representation of the term
  virtual const char *text();
};

/**
 * @brief Variable Term
 */
class YAPVarTerm: public YAPTerm {
  YAPVarTerm(Term t) { if (IsVarTerm(t)) mk( t ); }
public:
  /// constructor
  YAPVarTerm();
  /// get the internal representation
  CELL *getVar() { return VarOfTerm( gt() ); }
  /// is the variable bound to another one
  bool unbound() { return IsUnboundVar(VarOfTerm( gt() )); }
  virtual bool isVar() { return true; }   /// type check for unbound
  virtual bool isAtom() { return false; } ///  type check for atom
  virtual bool isInteger() { return false; } /// type check for integer
  virtual bool isFloat() { return false; } /// type check for floating-point
  virtual bool isString() { return false; } /// type check for a string " ... "
  virtual bool isCompound() { return false; } /// is a primitive term
  virtual bool isAppl() { return false; } /// is a structured term
  virtual bool isPair() { return false; } /// is a pair term
  virtual bool isGround() { return false; } /// term is ground
  virtual bool isList() { return false; } /// term is a list
};

/**
 * @brief Atom Term
 * Term Representation of an Atom
 */
class YAPAtomTerm: public YAPTerm {
  friend class YAPModule;
  // Constructor: receives a C-atom;
  YAPAtomTerm(Atom a)  { mk( MkAtomTerm(a) ); }
  YAPAtomTerm(Term t): YAPTerm(t)  { IsAtomTerm(t); }
  // Getter for Prolog atom
  Term getTerm() { return t; }
public:
  // Constructor: receives an atom;
  YAPAtomTerm(YAPAtom a): YAPTerm() { mk( MkAtomTerm(a.a) ); }
  // Constructor: receives a sequence of ISO-LATIN1 codes;
  YAPAtomTerm(char *s) ;
  // Constructor: receives a sequence of up to n ISO-LATIN1 codes;
  YAPAtomTerm(char *s, size_t len);
  // Constructor: receives a sequence of wchar_ts, whatever they may be;
  YAPAtomTerm(wchar_t *s) ;
  // Constructor: receives a sequence of n wchar_ts, whatever they may be;
  YAPAtomTerm(wchar_t *s, size_t len);
  virtual bool isVar() { return false; }   /// type check for unbound
  virtual bool isAtom() { return true; } ///  type check for atom
  virtual bool isInteger() { return false; } /// type check for integer
  virtual bool isFloat() { return false; } /// type check for floating-point
  virtual bool isString() { return false; } /// type check for a string " ... "
  virtual bool isCompound() { return false; } /// is a primitive term
  virtual bool isAppl() { return false; } /// is a structured term
  virtual bool isPair() { return false; } /// is a pair term
  virtual bool isGround() { return true; } /// term is ground
  virtual bool isList() { return gt() == TermNil; } /// [] is a list
  // Getter: outputs the atom;
  YAPAtom getAtom() { return YAPAtom(AtomOfTerm( gt() )); }
  // Getter: outputs the name as a sequence of ISO-LATIN1 codes;
  const char *text() { return (const char *)AtomOfTerm( gt() )->StrOfAE; }
};

/**
 * @brief Compound Term
 */
class YAPApplTerm: public YAPTerm {
  friend class YAPTerm;
  YAPApplTerm(Term t0) { mk(t0); }
public:
  ~YAPApplTerm() {  }
  YAPApplTerm(YAPFunctor f, YAPTerm ts[]);
  YAPApplTerm(YAPFunctor f);
  YAPFunctor getFunctor();
  YAPTerm getArg(int i);
  virtual bool isVar() { return false; }   /// type check for unbound
  virtual bool isAtom() { return false; } ///  type check for atom
  virtual bool isInteger() { return false; } /// type check for integer
  virtual bool isFloat() { return false; } /// type check for floating-point
  virtual bool isString() { return false; } /// type check for a string " ... "
  virtual bool isCompound() { return true; } /// is a primitive term
  virtual bool isAppl() { return true; } /// is a structured term
  virtual bool isPair() { return false; } /// is a pair term
  virtual bool isGround() { return true; } /// term is ground
  virtual bool isList() { return false; } /// [] is a list
};

/**
 * @brief List Constructor Term
 */
class YAPPairTerm: public YAPTerm {
  friend class YAPTerm;
  YAPPairTerm(Term t0) { if (IsPairTerm(t0)) mk( t0 );  else mk(0); }
public:
  YAPPairTerm(YAPTerm hd, YAPTerm tl);
  YAPPairTerm();
  YAPTerm getHead() { return  YAPTerm(HeadOfTerm( gt() )); }
  YAPTerm getTail() { return  YAPTerm(TailOfTerm( gt() )); }
};

/**
 * @brief Number Term
 */

class YAPNumberTerm: public YAPTerm {
public:
  YAPNumberTerm() {};
  bool isTagged() { return IsIntTerm( gt() ); }
};

/**
 * @brief Integer Term
 */

class YAPIntegerTerm: public YAPNumberTerm {
public:
  YAPIntegerTerm(intptr_t i);
  intptr_t getInteger() { return  IntegerOfTerm( gt() ); }
};

class YAPListTerm: public YAPTerm {
public:
  /// Create a list term out of a standard term. Check if a valid operation.
  ///
  /// @param[in] the term
  YAPListTerm() { mk(TermNil); /* else type_error */ }
  /// Create an empty list term.
  ///
  /// @param[in] the term
  YAPListTerm(Term t0) { mk(t0); /* else type_error */ }
  /*  /// Create a list term out of an array of terms.
 ///
 /// @param[in] the array of terms
 /// @param[in] the length of the array
 YAPListTerm(YAPTerm ts[], size_t n);
  */
  //      YAPListTerm( vector<YAPTerm> v );
  /// Return the number of elements in a list term.
  size_t length() { Term *tailp; Term t1 = gt(); return Yap_SkipList(&t1, &tailp); }
  /// Extract the first element of a list.
  ///
  /// @param[in] the list
  YAPTerm car();
  /// Extract the tail elements of a list.
  ///
  /// @param[in] the list
  YAPListTerm cdr()
  {
    Term to = gt();
    if (IsPairTerm( to ))
      return YAPListTerm(TailOfTerm( to ));
    else if ( to == TermNil)
      return YAPListTerm(  );
    /* error */
    throw YAPError(TYPE_ERROR_LIST);
  }

  /// Check if the list is empty.
  ///
  /// @param[in] the list
  inline bool nil() {
      { LOG(  "II %x %x", gt(), TermNil) ; }
    return gt() == TermNil;
  }

;
};

/**
 * @brief String Term
 */
class YAPStringTerm: public YAPTerm {
public:
  /// your standard constructor
  YAPStringTerm(char *s) ;
  /// use this one to construct length limited strings
  YAPStringTerm(char *s, size_t len);
  /// construct using wide chars
  YAPStringTerm(wchar_t *s) ;
  /// construct using length-limited wide chars
  YAPStringTerm(wchar_t *s, size_t len);
  const char *getString() { return StringOfTerm( gt() ); }
};

#endif /* YAPT_HH */
