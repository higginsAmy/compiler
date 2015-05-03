/* Semantics.h
   The action and supporting routines for performing semantics processing.
*/

#include <stdbool.h>

/* Semantic Records */
struct IdList {
  struct SymEntry *TheEntry;
  struct IdList *Next;
};

struct ExprRes {
  int Reg;
  struct InstrSeq *Instrs;
  bool isBool;
};

struct ExprResList {
  struct ExprRes *Expr;
  struct ExprResList *Next;
};

struct BExprRes {
  char *Label;
  int Reg;
  struct InstrSeq *Instrs;
};


/* Semantics Actions */
extern struct ExprRes *doConvert(struct BExprRes *Res);
extern struct ExprRes *doIntLit(char *digits);
extern struct BExprRes *doBLit(bool b);
extern struct ExprRes *doRval(char *name);
extern struct BExprRes *doNOT(struct BExprRes *Res);
extern struct BExprRes *doNOTe(struct ExprRes *Res);
extern struct InstrSeq *doAssign(char *name,  struct ExprRes *Res1);
extern struct InstrSeq *doBAssign(char *name, struct BExprRes *res);
extern struct ExprRes *doAdd(struct ExprRes *Res1,  struct ExprRes *Res2);
extern struct ExprRes *doSub(struct ExprRes *Res1, struct ExprRes *Res2);
extern struct ExprRes *doMult(struct ExprRes *Res1,  struct ExprRes *Res2);
extern struct ExprRes *doDiv(struct ExprRes *Res1, struct ExprRes *Res2);
extern struct ExprRes *doMod(struct ExprRes *Res1, struct ExprRes *Res2);
extern struct ExprRes *doExp(struct ExprRes *Res1, struct ExprRes *Res2);
extern struct ExprRes *doNEG(struct ExprRes *Res1);
extern struct InstrSeq *doPrint(struct ExprRes *Expr);
extern struct InstrSeq *doPrintList(struct ExprResList *List);
extern struct InstrSeq *doPrintLN();
extern struct InstrSeq *doPrintSP(struct ExprRes *Expr);
extern struct InstrSeq *doPrintSTR(char *string);
extern struct BExprRes *doBExpr(char *op, struct ExprRes *Res1,  struct ExprRes *Res2);
extern struct BExprRes *doINEQ(char *op, struct ExprRes *Res1, struct ExprRes *Res2);
extern struct BExprRes *doOR(struct ExprRes *Res1, struct ExprRes *Res2);
extern struct BExprRes *doAND(struct ExprRes *Res1, struct ExprRes *Res2);
extern struct InstrSeq *doIf(struct BExprRes *bRes, struct InstrSeq *seq);
extern struct InstrSeq *doIfElse(struct BExprRes *bRes, struct InstrSeq *seq, struct InstrSeq *seq2);
extern struct InstrSeq *doWhile(struct ExprRes *bRes, struct InstrSeq *seq);
extern struct ExprResList *doList(struct ExprRes *Res, struct ExprResList *ResList);
extern struct ExprResList *doListItem(struct ExprRes *Res);
extern struct InstrSeq *dupStruct(struct InstrSeq *Instrs);
extern void printExprList(struct ExprResList *list);
extern void Finish(struct InstrSeq *Code);
