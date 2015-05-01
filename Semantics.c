/* Semantics.c
   Support and semantic action routines.   
*/

#include <string.h>
#include <stdlib.h>
#include "CodeGen.h"
#include "Semantics.h"
#include "SymTab.h"
#include "IOMngr.h"

extern struct SymTab *table;

/* Semantics support routines */
extern struct ExprRes *doConvert(struct BExprRes *Res){
  struct ExprRes *eRes;

  eRes = (struct ExprRes *)malloc(sizeof(struct ExprRes));
  eRes->Instrs = Res->Instrs;
  eRes->Reg = Res->Reg;
  eRes->isBool = true;
  free(Res);

  return eRes;
}

struct ExprRes *doIntLit(char *digits){
  struct ExprRes *res;

  res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  res->Reg = AvailTmpReg();
  res->Instrs = GenInstr(NULL,"li",TmpRegName(res->Reg),digits,NULL);
  res->isBool = false;
  
  return res;
}

struct BExprRes *doBLit(bool b){
  struct BExprRes *bRes;

  bRes = (struct BExprRes *) malloc(sizeof(struct BExprRes));
  bRes->Label = GenLabel();
  bRes->Reg = AvailTmpReg();
  if (b){
    bRes->Instrs = GenInstr(NULL, "li", TmpRegName(bRes->Reg), "1", NULL);
  }
  else{
    bRes->Instrs = GenInstr(NULL, "li", TmpRegName(bRes->Reg), "0", NULL);
  }
  
  return bRes;
}

struct ExprRes *doRval(char *name){ 
  struct SymEntry *entry = FindName(table, name);
  struct ExprRes *Res;
  
  if (NULL == entry) {
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  Res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  Res->Reg = AvailTmpReg();
  Res->Instrs = GenInstr(NULL,"lw",TmpRegName(Res->Reg),name,NULL);
  if (0 == strcmp((char *)GetAttr(entry), "bool") && NULL != entry){
    Res->isBool = true;
    printf("Variable is a bool.\n");
  }
  else{
    Res->isBool = false;
    printf("Variable is not a bool.\n");
  }
  
  return Res;
}

struct BExprRes *doNOT(struct BExprRes *Res){
  int reg1 = AvailTmpReg();
  char *label = GenLabel();
  char *finish = GenLabel();

  AppendSeq(Res->Instrs, GenInstr(NULL, "li", TmpRegName(reg1), "1", NULL));
  AppendSeq(Res->Instrs, GenInstr(NULL, "beq", TmpRegName(Res->Reg),
				  TmpRegName(reg1), label));
  AppendSeq(Res->Instrs, GenInstr(NULL, "li", TmpRegName(Res->Reg), "1", NULL));
  AppendSeq(Res->Instrs, GenInstr(NULL, "j", finish, NULL, NULL));
  AppendSeq(Res->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));
  AppendSeq(Res->Instrs, GenInstr(NULL, "li", TmpRegName(Res->Reg), "0", NULL));
  AppendSeq(Res->Instrs, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg1);

  return Res;
}

struct BExprRes *doNOTe(struct ExprRes *Res){
  struct BExprRes *bRes;
  int reg1 = AvailTmpReg();
  char *label = GenLabel();
  char *finish = GenLabel();

  if (!Res->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply NOT operator to int.");
    exit(0);
  }

  bRes = (struct BExprRes *)malloc(sizeof(struct BExprRes));
  bRes->Instrs = Res->Instrs;
  bRes->Reg = Res->Reg;
  bRes->Label = GenLabel();
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(reg1), "1", NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "beq", TmpRegName(bRes->Reg),
				  TmpRegName(reg1), label));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(bRes->Reg), "1", NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "j", finish, NULL, NULL));
  AppendSeq(bRes->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(bRes->Reg), "0", NULL));
  AppendSeq(bRes->Instrs, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg1);
  free(Res);

  return bRes;
}

struct ExprRes *doAdd(struct ExprRes *Res1, struct ExprRes *Res2){ 
  int reg;

  if (Res1->isBool || Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply ADD operator to bool.");
    exit(0);
  }
  
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

  if (Res1->isBool || Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply SUB operator to bool.");
    exit(0);
  }
  
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

  if (Res1->isBool || Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply MUL operator to bool.");
    exit(0);
  }
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

  if (Res1->isBool || Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply DIV operator to bool.");
    exit(0);
  }
  
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

  if (Res1->isBool || Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply MOD operator to bool.");
    exit(0);
  }
  
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

  if (Res1->isBool || Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply MOD operator to bool.");
    exit(0);
  }
  
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

  if (Res1->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply NEG operator to bool.");
    exit(0);
  }
  
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

struct InstrSeq *doPrint(struct ExprRes *Expr){ 
  struct InstrSeq *code;
  int reg = AvailTmpReg();
  char *label;
  char *finish;
    
  code = Expr->Instrs;
  if (Expr->isBool){
    label = GenLabel();
    finish = GenLabel();
    printf("Printing a boolean\n");
    AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg), "1", NULL));
    AppendSeq(code, GenInstr(NULL, "li", "$v0", "4", NULL));
    AppendSeq(code, GenInstr(NULL, "bne", TmpRegName(Expr->Reg),
			     TmpRegName(reg), label));
    AppendSeq(code, GenInstr(NULL, "la", "$a0", "_t", NULL));
    AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
    AppendSeq(code, GenInstr(NULL, "j", finish, NULL, NULL));
    AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
    AppendSeq(code, GenInstr(NULL, "la", "$a0", "_f", NULL));
    AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
    AppendSeq(code, GenInstr(finish, NULL, NULL, NULL, NULL));
  }
  else{
    printf("Printing an int\n");
    AppendSeq(code,GenInstr(NULL,"li","$v0","1",NULL));
    AppendSeq(code,GenInstr(NULL,"move","$a0",TmpRegName(Expr->Reg),NULL));
    AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));
  }
  AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
  AppendSeq(code,GenInstr(NULL,"la","$a0","_nl",NULL));
  AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));
  ReleaseTmpReg(reg);
  ReleaseTmpReg(Expr->Reg);
  free(Expr);

  return code;
}

extern struct InstrSeq *doPrintList(struct ExprResList *List){
  struct InstrSeq *code;
  struct ExprRes *temp;
  int reg = AvailTmpReg();
  char *label;
  char *finish;

  //printExprList(List);
  code = GenInstr(NULL, NULL, NULL, NULL, NULL);
  while (List){
    temp = List->Expr;
    AppendSeq(code, temp->Instrs);
    if (temp->isBool){
      label = GenLabel();
      finish = GenLabel();
      printf("Printing a boolean\n");
      AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg), "1", NULL));
      AppendSeq(code, GenInstr(NULL, "li", "$v0", "4", NULL));
      AppendSeq(code, GenInstr(NULL, "bne", TmpRegName(temp->Reg),
    			       TmpRegName(reg), label));
      AppendSeq(code, GenInstr(NULL, "la", "$a0", "_t", NULL));
      AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
      AppendSeq(code, GenInstr(NULL, "j", finish, NULL, NULL));
      AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
      AppendSeq(code, GenInstr(NULL, "la", "$a0", "_f", NULL));
      AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
      AppendSeq(code, GenInstr(finish, NULL, NULL, NULL, NULL));
    }
    else{
      printf("Printing an int\n");
      AppendSeq(code, GenInstr(NULL,"li","$v0","1",NULL));
      AppendSeq(code, GenInstr(NULL,"move","$a0",TmpRegName(temp->Reg),NULL));
      AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));
    }
    AppendSeq(code, GenInstr(NULL,"li","$v0","4",NULL));
    AppendSeq(code, GenInstr(NULL,"la","$a0","_sp",NULL));
    AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));
    ReleaseTmpReg(temp->Reg);
    free(temp);
    List = List->Next;
  }
  AppendSeq(code, GenInstr(NULL,"li","$v0","4",NULL));
  AppendSeq(code, GenInstr(NULL,"la","$a0","_nl",NULL));
  AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));
  ReleaseTmpReg(reg);
  free(List);
  
  return code;
}

extern struct InstrSeq *doPrintLN(){
  struct InstrSeq *code;

  code = GenInstr(NULL,"li","$v0","4",NULL);
  AppendSeq(code, GenInstr(NULL,"la","$a0","_nl",NULL));
  AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));

  return code;
}

extern struct InstrSeq *doPrintSP(struct ExprRes *Expr){
  struct InstrSeq *code;
  char *loop = GenLabel();
  char *finish = GenLabel();

  code = Expr->Instrs;
  AppendSeq(code, GenInstr(loop, "blez", TmpRegName(Expr->Reg), finish, NULL));
  AppendSeq(code, GenInstr(NULL,"li","$v0","4",NULL));
  AppendSeq(code, GenInstr(NULL,"la","$a0","_sp",NULL));
  AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));
  AppendSeq(code, GenInstr(NULL, "addi", TmpRegName(Expr->Reg), TmpRegName(Expr->Reg), "-1"));
  AppendSeq(code, GenInstr(NULL, "j", loop, NULL, NULL));
  AppendSeq(code, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(Expr->Reg);
  free(Expr);

  return code;
}

struct InstrSeq *doAssign(char *name, struct ExprRes *Expr){ 
  struct InstrSeq *code;
  
  if (!FindName(table, name)) {
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  
  code = Expr->Instrs;
  AppendSeq(code, GenInstr(NULL, "sw", TmpRegName(Expr->Reg), name, NULL));
  ReleaseTmpReg(Expr->Reg);
  free(Expr);
  
  return code;
}

extern struct InstrSeq *doBAssign(char *name, struct BExprRes *Res){
  struct InstrSeq *code;
  
  if (!FindName(table, name)){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  
  code = Res->Instrs;
  AppendSeq(code, GenInstr(NULL, "sw", TmpRegName(Res->Reg), name, NULL));
  ReleaseTmpReg(Res->Reg);
  free(Res);
  
  return code;
}

extern struct BExprRes *doBExpr(char *op, struct ExprRes *Res1,  struct ExprRes *Res2){
  struct BExprRes * bRes;

  if (Res1->isBool || Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply INEQ operator to bool.");
    exit(0);
  }
  
  bRes = (struct BExprRes *) malloc(sizeof(struct BExprRes));
  bRes->Label = GenLabel();
  bRes->Reg = AvailTmpReg();
  AppendSeq(Res1->Instrs, Res2->Instrs);
  AppendSeq(Res1->Instrs, GenInstr(NULL, op, TmpRegName(bRes->Reg),
				   TmpRegName(Res1->Reg), TmpRegName(Res2->Reg)));
  bRes->Instrs = Res1->Instrs;
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  free(Res1);
  free(Res2);

  return bRes;
}

extern struct BExprRes *doOR(struct BExprRes *Res1, struct BExprRes *Res2){
  char *label = GenLabel();
  char *finish = GenLabel();
  int reg1 = AvailTmpReg();
  
  AppendSeq(Res1->Instrs, Res2->Instrs);
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li", TmpRegName(reg1), "1", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "beq", TmpRegName(Res1->Reg),
				   TmpRegName(reg1), label));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "bne", TmpRegName(Res2->Reg),
				   TmpRegName(reg1), Res2->Label));
  AppendSeq(Res1->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li", TmpRegName(Res1->Reg), "1", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "j", finish, NULL, NULL));
  AppendSeq(Res1->Instrs, GenInstr(Res2->Label, NULL, NULL, NULL, NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li", TmpRegName(Res1->Reg), "0", NULL));
  AppendSeq(Res1->Instrs, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg1);
  ReleaseTmpReg(Res2->Reg);
  free(Res2);
  
  return Res1;
}

extern struct BExprRes *doAND(struct BExprRes *Res1, struct BExprRes *Res2){
  char *label = GenLabel();
  char *finish = GenLabel();
  int reg1 = AvailTmpReg();
  
  AppendSeq(Res1->Instrs, Res2->Instrs);
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li", TmpRegName(reg1), "1", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "bne", TmpRegName(Res1->Reg),
				   TmpRegName(reg1), label));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "bne", TmpRegName(Res2->Reg),
				   TmpRegName(reg1), label));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li", TmpRegName(Res1->Reg), "1", NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "j", finish, NULL, NULL));
  AppendSeq(Res1->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));
  AppendSeq(Res1->Instrs, GenInstr(NULL, "li", TmpRegName(Res1->Reg), "0", NULL));
  AppendSeq(Res1->Instrs, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg1);
  ReleaseTmpReg(Res2->Reg);
  free(Res2);

  return Res1;
}

extern struct InstrSeq *doIf(struct BExprRes *bRes, struct InstrSeq * seq){
  struct InstrSeq * seq2;
  int reg = AvailTmpReg();
  
  seq2 = bRes->Instrs;
  AppendSeq(seq2, GenInstr(NULL, "li", TmpRegName(reg), "1", NULL));
  AppendSeq(seq2, GenInstr(NULL, "bne", TmpRegName(bRes->Reg),
			   TmpRegName(reg), bRes->Label));
  AppendSeq(seq2, seq);
  AppendSeq(seq2, GenInstr(bRes->Label, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg);
  ReleaseTmpReg(bRes->Reg);
  free(bRes);

  return seq2;
}

extern struct InstrSeq *doIfElse(struct BExprRes *bRes, struct InstrSeq *seq, struct InstrSeq *seq2){
  struct InstrSeq *code;
  char *finish = GenLabel();
  int reg = AvailTmpReg();

  code = bRes->Instrs;
  AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg), "1", NULL));
  AppendSeq(code, GenInstr(NULL, "bne", TmpRegName(bRes->Reg),
			   TmpRegName(reg), bRes->Label));
  AppendSeq(code, seq);
  AppendSeq(code, GenInstr(NULL, "j", finish, NULL, NULL));
  AppendSeq(code, GenInstr(bRes->Label, NULL, NULL, NULL, NULL));
  AppendSeq(code, seq2);
  AppendSeq(code, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg);
  ReleaseTmpReg(bRes->Reg);
  free(bRes);

  return code;
}

extern struct InstrSeq *doWhile(struct BExprRes *bRes, struct InstrSeq *seq){
  struct InstrSeq *code;
  char *loop = GenLabel();
  int reg = AvailTmpReg();

  code = bRes->Instrs;
  AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg), "1", NULL));
  AppendSeq(code, GenInstr(loop, "bne", TmpRegName(bRes->Reg),
			   TmpRegName(reg), bRes->Label));
  AppendSeq(code, seq);
  //AppendSeq(code, bRes->Instrs);
  //AppendSeq(code, GenInstr(NULL, "j", loop, NULL, NULL));
  AppendSeq(code, GenInstr(bRes->Label, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg);
  ReleaseTmpReg(bRes->Reg);
  free(bRes);
  free(seq);
  
  return code;
}

extern struct ExprResList *doList(struct ExprRes *Res, struct ExprResList *ResList){
  struct ExprResList *list;

  list = (struct ExprResList *)malloc(sizeof(struct ExprResList));
  list->Expr = Res;
  list->Next = ResList;

  return list;
}

extern struct ExprResList *doListItem(struct ExprRes *Res){
  struct ExprResList *list;

  list = (struct ExprResList *)malloc(sizeof(struct ExprResList));
  list->Expr = Res;
  list->Next = NULL;

  return list;
}

void printExprList(struct ExprResList *list){
  struct ExprResList *templist;
  struct ExprRes *temp;

  while (templist){
    temp = templist->Expr;
    if (temp->isBool){
      printf("Variable is type: boolean\n");
    }
    else{
      printf("Variable is type: int\n");
    }
    templist = templist->Next;
  }
}

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
  AppendSeq(code, GenInstr("_nl", ".asciiz", "\"\\n\"",NULL,NULL));
  AppendSeq(code, GenInstr("_sp", ".asciiz", "\" \"", NULL, NULL));
  AppendSeq(code, GenInstr("_t", ".asciiz", "\"true\"", NULL, NULL));
  AppendSeq(code, GenInstr("_f", ".asciiz", "\"false\"", NULL, NULL));
  AppendSeq(code, GenInstr("errExp", ".asciiz",
			   "\"error: your exponent cannot be less than 0.\"",NULL,NULL));
  
  entry = FirstEntry(table);
  while (entry) {
   AppendSeq(code,GenInstr((char *) GetName(entry),".word","0",NULL,NULL));
   entry = NextEntry(table, entry);
  }
  
  WriteSeq(code);
  
  return;
}
