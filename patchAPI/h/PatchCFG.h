/* Public Interface */

#ifndef _PATCHAPI_DYNINST_CFG_H_
#define _PATCHAPI_DYNINST_CFG_H_

#include "common.h"
#include "PatchObject.h"

namespace Dyninst {
namespace PatchAPI {

class PatchEdge;
class PatchBlock;
class PatchFunction;
class PatchObject;

/* PatchAPI Edge */
class PatchEdge {
   friend class PatchBlock;
   friend class PatchFunction;
   friend class PatchObject;

  public:
   static PatchEdge *create(ParseAPI::Edge *, PatchBlock *src, PatchBlock *trg);
   PatchEdge(ParseAPI::Edge *internalEdge,
             PatchBlock *source,
             PatchBlock *target);

   PatchEdge(const PatchEdge *parent,
             PatchBlock *child_src,
             PatchBlock *child_trg);
   ~PatchEdge();

   // Getters
   ParseAPI::Edge *edge() const;
   PatchBlock *source();
   PatchBlock *target();
   ParseAPI::EdgeTypeEnum type() const;
   bool sinkEdge() const;
   bool interproc() const;

 protected:
    ParseAPI::Edge *edge_;
    PatchBlock *src_;
    PatchBlock *trg_;
};

/* This is somewhat mangled, but allows PatchAPI to access the
   iteration predicates of ParseAPI without having to go back and
   template that code. Just wrap a ParseAPI predicate in a
   EdgePredicateAdapter and *poof* you're using PatchAPI edges
   instead of ParseAPI edges... */
class EdgePredicateAdapter
   : public ParseAPI::iterator_predicate <
  EdgePredicateAdapter,
  PatchEdge *,
  PatchEdge * > {
  public:
    EdgePredicateAdapter() : int_(NULL) {};
    EdgePredicateAdapter(ParseAPI::EdgePredicate *intPred) : int_(intPred) {};
    virtual ~EdgePredicateAdapter() {};
     virtual bool pred_impl(PatchEdge *e) const { return int_->pred_impl(e->edge()); };

  private:
    ParseAPI::EdgePredicate *int_;
};

/* PatchAPI Block */
class PatchBlock {
  friend class PatchEdge;
  friend class PatchFunction;
  friend class PatchObject;

  public:
    typedef std::map<Address, InstructionAPI::Instruction::Ptr> Insns;
    typedef std::vector<PatchEdge*> edgelist;

    static PatchBlock *create(ParseAPI::Block *, PatchFunction *);
    PatchBlock(const PatchBlock *parblk, PatchObject *child);
    PatchBlock(ParseAPI::Block *block, PatchObject *obj);
    virtual ~PatchBlock();

    // Getters
    Address start() const;
    Address end() const;
    Address last() const;
    Address size() const;

    bool isShared();
    int containingFuncs() const;
    void getInsns(Insns &insns) const;
    InstructionAPI::Instruction::Ptr getInsn(Address a) const;
    std::string disassemble() const;
    bool containsCall();
    bool containsDynamicCall();
    std::string format() const;

    // Difference between this layer and ParseAPI: per-function blocks.
    PatchFunction *function() const { return function_; }
    ParseAPI::Block *block() const { return block_; }
    PatchObject* object() const { return obj_; }
    edgelist &getSources();
    edgelist &getTargets();


  protected:
    typedef enum {
      backwards,
      forwards } Direction;

    void removeSourceEdge(PatchEdge *e);
    void removeTargetEdge(PatchEdge *e);
    void createInterproceduralEdges(ParseAPI::Edge *, Direction dir,
                                    std::vector<PatchEdge *> &);

    ParseAPI::Block *block_;
    PatchFunction *function_;
    edgelist srclist_;
    edgelist trglist_;
    PatchObject* obj_;
};

/* PatchAPI Function */
class PatchFunction {
   friend class PatchEdge;
   friend class PatchBlock;
   friend class PatchObject;

   public:
     typedef std::vector<PatchBlock *> blocklist;

     static PatchFunction *create(ParseAPI::Function *, PatchObject*);
     PatchFunction(ParseAPI::Function *f, PatchObject* o);
     PatchFunction(const PatchFunction* parFunc, PatchObject* child);
     virtual ~PatchFunction();

     const string &name() { return func_->name(); }
     Address addr() const { return addr_;  }
     ParseAPI::Function *function() { return func_; }
     PatchObject* object() { return obj_; }

     const blocklist &getAllBlocks();
     PatchBlock *getEntryBlock();
     const blocklist &getExitBlocks();
     const blocklist &getCallBlocks();

   protected:
     ParseAPI::Function *func_;
     PatchObject* obj_;
     Address addr_;

     blocklist all_blocks_;
     blocklist exit_blocks_;
     blocklist call_blocks_;
};

};
};


#endif /* _PATCHAPI_DYNINST_CFG_H_ */
