#ifndef HORUS_CFACTORGRAPH_H
#define HORUS_CFACTORGRAPH_H

#include <unordered_map>

#include "FactorGraph.h"
#include "Factor.h"
#include "Horus.h"

class VarCluster;
class FacCluster;
class Distribution;
class Signature;

class SignatureHash;


typedef long Color;

typedef unordered_map<unsigned, vector<Color>> VarColorMap;

typedef unordered_map<unsigned, Color>    DistColorMap;
typedef unordered_map<VarId, VarCluster*> VarId2VarCluster;

typedef vector<VarCluster*> VarClusterSet;
typedef vector<FacCluster*> FacClusterSet;

typedef unordered_map<Signature, VarNodes, SignatureHash> VarSignMap;
typedef unordered_map<Signature, FactorNodes, SignatureHash> FacSignMap;



struct Signature
{
  Signature (unsigned size) : colors(size) {  }

  bool operator< (const Signature& sig) const
  {
    if (colors.size() < sig.colors.size()) {
      return true;
    } else if (colors.size() > sig.colors.size()) {
      return false;
    } else {
      for (unsigned i = 0; i < colors.size(); i++) {
        if (colors[i] < sig.colors[i]) {
          return true;
        } else if (colors[i] > sig.colors[i]) {
          return false;
        }
      }
    }
    return false;
  }

  bool operator== (const Signature& sig) const
  {
    if (colors.size() != sig.colors.size()) {
      return false;
    }
    for (unsigned i = 0; i < colors.size(); i++) {
      if (colors[i] != sig.colors[i]) {
        return false;
      }
    }
    return true;
  }

  vector<Color> colors;
};



struct SignatureHash
{
  size_t operator() (const Signature &sig) const
  {
    size_t val = hash<size_t>()(sig.colors.size());
    for (unsigned i = 0; i < sig.colors.size(); i++) {
      val ^= hash<size_t>()(sig.colors[i]);
    }
    return val;
  }
};



class VarCluster
{
  public:
    VarCluster (const VarNodes& vs)
    {
      for (unsigned i = 0; i < vs.size(); i++) {
        groundVars_.push_back (vs[i]);
      }
    }

    void addFacCluster (FacCluster* fc)
    {
      facClusters_.push_back (fc);
    }

    const FacClusterSet& getFacClusters (void) const
    {
      return facClusters_;
    }

    VarNode* getRepresentativeVariable (void) const { return representVar_; }
    void setRepresentativeVariable (VarNode* v)     { representVar_ = v; }
    const VarNodes& getGroundVarNodes (void) const  { return groundVars_; }

  private:
    VarNodes        groundVars_;
    FacClusterSet   facClusters_;
    VarNode*      representVar_;
};


class FacCluster
{
  public:
    FacCluster (const FactorNodes& groundFactors, const VarClusterSet& vcs)
    {
      groundFactors_ = groundFactors;
      varClusters_ = vcs;
      for (unsigned i = 0; i < varClusters_.size(); i++) {
        varClusters_[i]->addFacCluster (this);
      }
    }
 
    const VarClusterSet& getVarClusters (void) const
    {
      return varClusters_;
    }
  
    bool containsGround (const FactorNode* fn)
    {
      for (unsigned i = 0; i < groundFactors_.size(); i++) {
        if (groundFactors_[i] == fn) {
          return true;
        }
      }
      return false;
    }

    FactorNode* getRepresentativeFactor (void) const
    {
      return representFactor_;
    }

    void setRepresentativeFactor (FactorNode* fn) 
    { 
      representFactor_ = fn;
    }

    const FactorNodes& getGroundFactors (void) const
    {
      return groundFactors_;
    }

 
  private:
    FactorNodes        groundFactors_;
    VarClusterSet   varClusters_;
    FactorNode*      representFactor_;
};


class CFactorGraph
{
  public:
    CFactorGraph (const FactorGraph&);

   ~CFactorGraph (void);

    const VarClusterSet& getVarClusters (void) { return varClusters_; }

    const FacClusterSet& getFacClusters (void) { return facClusters_; }

    VarNode* getEquivalentVariable (VarId vid)
    {
      VarCluster* vc = vid2VarCluster_.find (vid)->second;
      return vc->getRepresentativeVariable();
    }

    FactorGraph* getCompressedFactorGraph (void);

    unsigned getGroundEdgeCount (const FacCluster*, const VarCluster*) const;
 
    static bool checkForIdenticalFactors;
 
  private:
    Color getFreeColor (void)
    {
      ++ freeColor_;
      return freeColor_ - 1;
    }

    Color getColor (const VarNode* vn) const
    {
      return varColors_[vn->getIndex()];
    }
    Color getColor (const FactorNode* fn) const  {
      return factorColors_[fn->getIndex()];
    }

    void setColor (const VarNode* vn, Color c)
    {
      varColors_[vn->getIndex()] = c;
    }

    void setColor (const FactorNode* fn, Color  c)
    {
      factorColors_[fn->getIndex()] = c;
    }

    VarCluster* getVariableCluster (VarId vid) const
    {
      return vid2VarCluster_.find (vid)->second;
    }

    void  setInitialColors (void);

    void  createGroups (void);

    void  createClusters (const VarSignMap&, const FacSignMap&);

    const Signature&  getSignature (const VarNode*);

    const Signature&  getSignature (const FactorNode*);

    void  printGroups (const VarSignMap&, const FacSignMap&) const;

    Color               freeColor_;
    vector<Color>       varColors_;
    vector<Color>       factorColors_;
    vector<Signature>   varSignatures_;
    vector<Signature>   factorSignatures_;
    VarClusterSet       varClusters_;
    FacClusterSet       facClusters_;
    VarId2VarCluster    vid2VarCluster_;
    const FactorGraph*  groundFg_;
};

#endif // HORUS_CFACTORGRAPH_H

