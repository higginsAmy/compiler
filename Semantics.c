/* Semantics.c
   Support and semantic action routines.   
*/

#include <strings.h>
#include <stdlib.h>
#include "CodeGen.h"
#include "Semantics.h"
#include "SymTab.h"
#include "IOMngr.h"

extern struct SymTab *table;

/* Semantics support routines */

struct ExprRes *doIntLit(char * digits){
  struct ExprRes *res;

  res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  res->Reg = AvailTmpReg();
  res->Instrs = GenInstr(NULL,"li",TmpRegName(res->Reg),digits,NULL);

  return res;
}

struct ExprRes *doRval(char *name){ 
  struct ExprRes *res;
  
  if (!FindName(table, name)) {
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  res->Reg = AvailTmpReg();
  res->Instrs = GenInstr(NULL,"lw",TmpRegName(res->Reg),name,NULL);

  return res;
}

struct BExprRes *doBval(char *name){
  struct ExprRes *res;
  struct ExprRes *res2;
  
  if (!FindName(table, name)){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  res->Reg = AvailTmpReg();
  res->Instrs = GenInstr(NULL, "lw", TmpRegName(res->Reg), name, NULL);
  res2 = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  res2->Reg = AvailTmpReg();
  AppendSeq(res2->Instrs, GenInstr(NULL, "li",
				   TmpRegName(res2->Reg), "1", NULL));

  return doBExpr("bne", res, res2);
}

struct ExprRes *doAdd(struct ExprRes *Res1, struct ExprRes *Res2){ 
  int reg;
   
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"add",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);
  
  return Res1;
}

struct ExprRes *doSub(struct ExprRes * Res1, struct ExprRes * Res2){
  int reg;

  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"sub",
				  TmpRegName(reg),
				  TmpRegName(Res1->Reg),
				  TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);

  return Res1;
}

struct ExprRes *doMult(struct ExprRes * Res1, struct ExprRes * Res2){ 
  int reg;
   
  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"mul",
                                       TmpRegName(reg),
                                       TmpRegName(Res1->Reg),
                                       TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);
  
  return Res1;
}

struct ExprRes *doDiv(struct ExprRes * Res1, struct ExprRes * Res2){
  int reg;

  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"div",
				  TmpRegName(reg),
				  TmpRegName(Res1->Reg),
				  TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);

  return Res1;
}

struct ExprRes *doMod(struct ExprRes * Res1, struct ExprRes * Res2){
  int reg;

  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs,Res2->Instrs);
  AppendSeq(Res1->Instrs,GenInstr(NULL,"rem",
				  TmpRegName(reg),
				  TmpRegName(Res1->Reg),
				  TmpRegName(Res2->Reg)));
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  Res1->Reg = reg;
  free(Res2);

  return Res1;
}

struct ExprRes *doExp(struct ExprRes *Res1, struct ExprRes *Res2){
  int reg, reg2;
  char * loop = GenLabel();
  char * err = GenLabel();
  char * zero = GenLabel();
  char * fin = GenLabel();
  
  reg = AvailTmpReg();
  reg2 = AvailTmpReg();
  AppendSeq(Res1->Instrs, Res2->Instrs);
 
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li",
				   TmpRegName(reg), "0", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "blt",
				   TmpRegName(Res2->Reg),
				   TmpRegName(reg), err));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "beq",
				   TmpRegName(Res2->Reg),
				   TmpRegName(reg), zero));
  // begin loop if exponent is > 0
  AppendSeq(Res1->Instrs, GenInstr(NULL, "addi",
				   TmpRegName(reg2),
				   TmpRegName(Res1->Reg), "0"));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "addi",
				   TmpRegName(reg),
				   TmpRegName(reg), "1"));
  AppendSeq(Res1->Instrs, GenInstr(loop, "beq",
				   TmpRegName(reg),
				   TmpRegName(Res2->Reg), fin));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "mul",
				   TmpRegName(Res1->Reg),
				   TmpRegName(Res1->Reg),
				   TmpRegName(reg2)));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "addi",
				   TmpRegName(reg),
				   TmpRegName(reg), "1"));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "j", loop, NULL, NULL));
  
  // print error statement if exponent less than 0, set result to 0.
  AppendSeq(Res1->Instrs, GenInstr(err, "li","$v0","4",NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL,"la","$a0", "errExp", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL,"syscall",NULL,NULL,NULL));

  AppendSeq(Res1->Instrs, GenInstr(NULL,"li","$v0","4",NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL,"la","$a0","_nl",NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL,"syscall",NULL,NULL,NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li",
				   TmpRegName(Res1->Reg), "0", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "j", fin, NULL, NULL));

  // if exponent is 0, set result to 1.
  AppendSeq(Res1->Instrs, GenInstr(zero, "li",
				   TmpRegName(Res1->Reg), "1", NULL));

  AppendSeq(Res1->Instrs, GenInstr(fin, NULL, NULL, NULL, NULL));

  ReleaseTmpReg(Res2->Reg);
  ReleaseTmpReg(reg);
  ReleaseTmpReg(reg2);
  free(Res2);
  
  return Res1;
}

struct ExprRes *doNEG(struct ExprRes *Res1){
  int reg;

  reg = AvailTmpReg();
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li",
				   TmpRegName(reg), "-1", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "mul", 
				   TmpRegName(reg), 
				   TmpRegName(reg),
				   TmpRegName(Res1->Reg)));
  ReleaseTmpReg(Res1->Reg);
  Res1->Reg = reg;

  return Res1;
}

struct InstrSeq *doPrint(struct ExprRes * Expr){ 
  struct InstrSeq *code;
    
  code = Expr->Instrs;
  
  AppendSeq(code,GenInstr(NULL,"li","$v0","1",NULL));
  AppendSeq(code,GenInstr(NULL,"move","$a0",TmpRegName(Expr->Reg),NULL));
  AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));

  AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
  AppendSeq(code,GenInstr(NULL,"la","$a0","_nl",NULL));
  AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));

  ReleaseTmpReg(Expr->Reg);
  free(Expr);

  return code;
}

struct InstrSeq *doAssign(char *name, struct ExprRes * Expr){ 
  struct InstrSeq *code;

  if (!FindName(table, name)) {
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }

  code = Expr->Instrs;
  
  AppendSeq(code,GenInstr(NULL,"sw",TmpRegName(Expr->Reg), name,NULL));

  ReleaseTmpReg(Expr->Reg);
  free(Expr);
  
  return code;
}

extern struct InstrSeq *doBAssign(char *name, bool boolean){
  struct InstrSeq *code;
  
  if (!FindName(table, name)){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  
  return code;
}

extern struct BExprRes *doBExpr(char *op, struct ExprRes *Res1,  struct ExprRes *Res2){
  struct BExprRes * bRes;

  bRes = (struct BExprRes *) malloc(sizeof(struct BExprRes));
  bRes->Label = GenLabel();
  AppendSeq(Res1->Instrs, GenInstr(NULL, op, TmpRegName(Res1->Reg), TmpRegName(Res2->Reg), bRes->Label));
  bRes->Instrs = Res1->Instrs;
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  free(Res1);
  free(Res2);

  return bRes;
}

extern struct BExprRes *doINEQ(char *op, struct ExprRes *Res1, struct ExprRes *Res2){
  AppendSeq(Res1->Instrs, Res2->Instrs);
  AppendSeq(Res1->Instrs, GenInstr(NULL, op, TmpRegName(Res1->Reg), TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li", TmpRegName(Res2->Reg), "1", NULL));
  
  return doBExpr("bne", Res1, Res2);
}

extern struct BExprRes *doOR(struct BExprRes *Res1, struct BExprRes *Res2){

  AppendSeq(Res1->Instrs, Res2->Instrs);
  AppendSeq(Res1->Instrs, GenInstr(Res1->Label, NULL, NULL, NULL, NULL));
  Res1->Label = Res2->Label;
  free(Res2);

  return Res1;
}

extern struct BExprRes *doAND(struct BExprRes *Res1, struct BExprRes *Res2){
  
  AppendSeq(Res1->Instrs, Res2->Instrs);
  AppendSeq(Res1->Instrs, GenInstr(Res2->Label, NULL, NULL, NULL, NULL));
  free(Res2);

  return Res1;
}

extern struct InstrSeq *doIf(struct BExprRes *bRes, struct InstrSeq * seq){
  struct InstrSeq * seq2;

  seq2 = AppendSeq(bRes->Instrs, seq);
  AppendSeq(seq2, GenInstr(bRes->Label, NULL, NULL, NULL, NULL));
  free(bRes);

  return seq2;
}

/*
extern struct InstrSeq * doIf(struct ExprRes *res1, struct ExprRes *res2, struct InstrSeq * seq){
  struct InstrSeq *seq2;
  char * label;
  
  label = GenLabel();
  AppendSeq(res1->Instrs, res2->Instrs);
  AppendSeq(res1->Instrs, GenInstr(NULL, "bne", TmpRegName(res1->Reg), TmpRegName(res2->Reg), label));
  seq2 = AppendSeq(res1->Instrs, seq);
  AppendSeq(seq2, GenInstr(label, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(res1->Reg);
  ReleaseTmpReg(res2->Reg);
  free(res1);
  free(res2);

  return seq2;
}
*/

void Finish(struct InstrSeq *Code){
  struct InstrSeq *code;
  struct SymEntry *entry;
  //struct Attr * attr;

  code = GenInstr(NULL,".text",NULL,NULL,NULL);
  //AppendSeq(code,GenInstr(NULL,".align","2",NULL,NULL));
  AppendSeq(code, GenInstr(NULL,".globl","main",NULL,NULL));
  AppendSeq(code, GenInstr("main",NULL,NULL,NULL,NULL));
  AppendSeq(code, Code);
  AppendSeq(code, GenInstr(NULL, "li", "$v0", "10", NULL)); 
  AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));
  AppendSeq(code, GenInstr(NULL,".data",NULL,NULL,NULL));
  AppendSeq(code, GenInstr(NULL,".align","4",NULL,NULL));
  AppendSeq(code, GenInstr("_nl",".asciiz","\"\\n\"",NULL,NULL));
  AppendSeq(code, GenInstr("errExp",".asciiz",
			   "\"error: your exponent cannot be less than 0.\"",NULL,NULL));
  
  entry = FirstEntry(table);
  while (entry) {
   AppendSeq(code,GenInstr((char *) GetName(entry),".word","0",NULL,NULL));
   entry = NextEntry(table, entry);
  }
  
  WriteSeq(code);
  
  return;
}
