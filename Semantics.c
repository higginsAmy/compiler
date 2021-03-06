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
extern struct FunctList *fList;

/* Semantics support routines */
struct InstrSeq *doRead(struct IdList *List){
  struct InstrSeq *code;
  int reg = AvailTmpReg();
  int reg2 = AvailTmpReg();
  char *label;

  code = GenInstr(NULL, NULL, NULL, NULL, NULL);
  while (NULL != List){
    AppendSeq(code, doPrintSTR("\"Please enter a value.\""));
    AppendSeq(code, GenInstr(NULL, "addi", "$v0", "$zero", "5"));
    AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
    if ((0 == strcmp((char *)GetAttr(List->TheEntry), "bool[]")) ||
	(0 == strcmp((char *)GetAttr(List->TheEntry), "int[]"))){
	AppendSeq(code, List->Expr->Instrs);
	label = GenLabel();

	AppendSeq(code, GenInstr(NULL, "bltz", TmpRegName(List->Expr->Reg), label, NULL));
        AppendSeq(code, GenInstr(NULL, "lw", TmpRegName(reg), (char *)GetName(List->TheEntry), NULL));
        AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg2), "4", NULL));
	AppendSeq(code, GenInstr(NULL, "mul", TmpRegName(reg2), TmpRegName(List->Expr->Reg), TmpRegName(reg2)));
	AppendSeq(code, GenInstr(NULL, "add", TmpRegName(reg), TmpRegName(reg), TmpRegName(reg2)));

	char *regName = (char *)malloc(6 * sizeof(char));
	strcpy(regName, "(");
	strcat(regName, TmpRegName(reg));
	strcat(regName, ")");

        AppendSeq(code, GenInstr(NULL, "sw", "$v0", regName, NULL));
	AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
	
	ReleaseTmpReg(List->Expr->Reg);
	free(List->Expr);
	free(regName);
    }
    else{
      AppendSeq(code, GenInstr(NULL, "sw", "$v0", (char *)GetName(List->TheEntry), NULL));
    }
    List = List->Next;
  }
  
  ReleaseTmpReg(reg);
  ReleaseTmpReg(reg2);
  
  return code;
}

struct InstrSeq *enterArr(char *Id, struct ExprRes *Res, char *type){
  struct InstrSeq *code;
  struct SymEntry *entry;
  char *label = GenLabel();
  int reg = AvailTmpReg();
  
  if (Res->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Cannot construct an array with 'bool' quantity of elements.");
    exit(0);
  }
  
  if (!EnterName(table, Id, &entry)){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Array Id not entered into table.");
  }
  SetAttr(entry, (void *)type);
  char *name = (char *)malloc((strlen(Id) + 5) * sizeof(char));
  strcpy(name, "len_");
  strcat(name, Id);
  EnterName(table, name, &entry);
  SetAttr(entry, (void *)"int");

  char *name2 = (char *)malloc((strlen(name) + 5) * sizeof(char));
  strcpy(name2, "var_");
  strcat(name2, name);

  char *name3 = (char *)malloc((strlen(Id) + 5) * sizeof(char));
  strcpy(name3, "var_");
  strcat(name3, Id);
  
  code = Res->Instrs;
  AppendSeq(code, GenInstr(NULL, "sw", TmpRegName(Res->Reg), name2, NULL));
  AppendSeq(code, GenInstr(NULL, "blez", TmpRegName(Res->Reg), label, NULL));
  AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg), "4", NULL));
  AppendSeq(code, GenInstr(NULL, "mul", TmpRegName(Res->Reg), TmpRegName(Res->Reg), TmpRegName(reg)));
  AppendSeq(code, GenInstr(NULL, "move", "$a0", TmpRegName(Res->Reg), NULL));
  AppendSeq(code, GenInstr(NULL, "li", "$v0", "9", NULL));
  AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
  AppendSeq(code, GenInstr(NULL, "sw", "$v0", name3, NULL));
  AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg);
  ReleaseTmpReg(Res->Reg);
  free(Res);
  free(name);
  free(name2);
  free(name3);
  
  return code;
}

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
  char *Name = (char *)malloc((strlen(name)+5)*sizeof(char));

  strcpy(Name, "var_");
  strcat(Name, name);
  
  if (NULL == entry) {
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  Res = (struct ExprRes *) malloc(sizeof(struct ExprRes));
  Res->Reg = AvailTmpReg();
  Res->Instrs = GenInstr(NULL, "lw", TmpRegName(Res->Reg), Name, NULL);
  if (0 == strcmp((char *)GetAttr(entry), "bool") && NULL != entry){
    Res->isBool = true;
    //printf("Variable is a bool.\n");
  }
  else{
    Res->isBool = false;
    //printf("Variable is not a bool.\n");
  }
  
  return Res;
}

extern struct ExprRes *doArrVal(char *Id, struct ExprRes *Res){
  char *label = GenLabel();
  int reg = AvailTmpReg();
  int reg2 = AvailTmpReg();

  if (!FindName(table, Id)){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }

  char *Name = (char *)malloc((strlen(Id) + 5) * sizeof(char));
  strcpy(Name, "var_");
  strcat(Name, Id);

  char *name2 = (char *)malloc((strlen(Id) +9) * sizeof(char));
  strcpy(name2, "var_len_");
  strcat(name2, Id);

  AppendSeq(Res->Instrs, GenInstr(NULL, "bltz", TmpRegName(Res->Reg), label, NULL));
  AppendSeq(Res->Instrs, GenInstr(NULL, "lw", TmpRegName(reg), name2, NULL));
  AppendSeq(Res->Instrs, GenInstr(NULL, "bge", TmpRegName(Res->Reg), TmpRegName(reg), label));
  AppendSeq(Res->Instrs, GenInstr(NULL, "lw", TmpRegName(reg), Name, NULL));
  AppendSeq(Res->Instrs, GenInstr(NULL, "li", TmpRegName(reg2), "4", NULL));
  AppendSeq(Res->Instrs, GenInstr(NULL, "mul", TmpRegName(reg2), TmpRegName(Res->Reg), TmpRegName(reg2)));
  AppendSeq(Res->Instrs, GenInstr(NULL, "add", TmpRegName(reg), TmpRegName(reg), TmpRegName(reg2)));

  char *regName = (char *)malloc(6 * sizeof(char));
  strcpy(regName, "(");
  strcat(regName, TmpRegName(reg));
  strcat(regName, ")");

  AppendSeq(Res->Instrs, GenInstr(NULL, "lw", TmpRegName(Res->Reg), regName, NULL));
  AppendSeq(Res->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));

  ReleaseTmpReg(reg);
  ReleaseTmpReg(reg2);
  free(Name);
  free(name2);
  free(regName);

  return Res;
}

struct ExprRes *doNOT(struct ExprRes *Res){
  int reg1 = AvailTmpReg();
  char *label = GenLabel();
  char *finish = GenLabel();

  if (!Res->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply NOT operator to int.");
    exit(0);
  }
  
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
    //printf("Printing a boolean\n");
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
    //printf("Printing an int\n");
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
      //printf("Printing a boolean\n");
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
      //printf("Printing an int\n");
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

extern struct InstrSeq *doPrintSTR(char *string){
  int reg = AvailTmpReg();
  struct InstrSeq *code;
  struct SymEntry *entry;
  char *varName = GenLabel();
  char *newVar;
  char *newVarName;

  newVar = (char *)malloc((strlen(varName) + 5) * sizeof(char));
  strcpy(newVar, "str_");
  strcat(newVar, varName);
  EnterName(table, newVar, &entry);
  SetAttr(entry, "string");
  SetStrVal(entry, (void *)string);
  newVarName = (char *)malloc((strlen(varName) + 9) * sizeof(char));
  strcpy(newVarName, "var_str_");
  strcat(newVarName, varName);
  
  code = GenInstr(NULL,"li","$v0","4",NULL);
  AppendSeq(code, GenInstr(NULL,"la","$a0", newVarName, NULL));
  AppendSeq(code, GenInstr(NULL,"syscall",NULL,NULL,NULL));
  AppendSeq(code, GenInstr(NULL, "la", "$a0", "_nl", NULL));
  AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
  ReleaseTmpReg(reg);
  free(newVar);
  free(newVarName);

  return code;
}

extern struct InstrSeq *doPrintArr(char *Id, struct ExprRes *Res){
  struct SymEntry *entry = FindName(table, Id);
  struct InstrSeq *code;
  char *label = GenLabel();
  char *label2;
  char *finish;
  int reg = AvailTmpReg();
  int reg2 = AvailTmpReg();

  if (NULL == entry) {
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  char *name = (char *)malloc((strlen(Id) + 5) * sizeof(char));
  strcpy(name, "var_");
  strcat(name, Id);

  char *name2 = (char *)malloc((strlen(Id) + 9) * sizeof(char));
  strcpy(name2, "var_len_");
  strcat(name2, Id);

  code = Res->Instrs;
  AppendSeq(code, GenInstr(NULL, "lw", TmpRegName(reg), name2, NULL));
  AppendSeq(code, GenInstr(NULL, "bltz", TmpRegName(Res->Reg), label, NULL));
  AppendSeq(code, GenInstr(NULL, "bge", TmpRegName(Res->Reg), TmpRegName(reg), label));
  AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg), "4", NULL));
  AppendSeq(code, GenInstr(NULL, "mul", TmpRegName(reg), TmpRegName(reg), TmpRegName(Res->Reg)));
  AppendSeq(code, GenInstr(NULL, "lw", TmpRegName(reg2), name, NULL));
  AppendSeq(code, GenInstr(NULL, "add", TmpRegName(reg), TmpRegName(reg), TmpRegName(reg2)));
  
  char *regName = (char *)malloc(6 * sizeof(char));
  strcpy(regName, "(");
  strcat(regName, TmpRegName(reg));
  strcat(regName, ")");

  AppendSeq(code, GenInstr(NULL, "lw", TmpRegName(reg), regName, NULL));
  
  if (0 == strcmp((char*)GetAttr(entry), "bool[]")){
    label2 = GenLabel();
    finish = GenLabel();
    //printf("Printing a boolean\n");
    AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg2), "1", NULL));
    AppendSeq(code, GenInstr(NULL, "li", "$v0", "4", NULL));
    AppendSeq(code, GenInstr(NULL, "bne", TmpRegName(reg), TmpRegName(reg2), label2));
    AppendSeq(code, GenInstr(NULL, "la", "$a0", "_t", NULL));
    AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
    AppendSeq(code, GenInstr(NULL, "j", finish, NULL, NULL));
    AppendSeq(code, GenInstr(label2, NULL, NULL, NULL, NULL));
    AppendSeq(code, GenInstr(NULL, "la", "$a0", "_f", NULL));
    AppendSeq(code, GenInstr(NULL, "syscall", NULL, NULL, NULL));
    AppendSeq(code, GenInstr(finish, NULL, NULL, NULL, NULL));
  }
  else{
    AppendSeq(code,GenInstr(NULL,"li","$v0","1",NULL));
    AppendSeq(code,GenInstr(NULL,"move","$a0",TmpRegName(reg),NULL));
    AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));
  }
  AppendSeq(code,GenInstr(NULL,"li","$v0","4",NULL));
  AppendSeq(code,GenInstr(NULL,"la","$a0","_nl",NULL));
  AppendSeq(code,GenInstr(NULL,"syscall",NULL,NULL,NULL));
  AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
  
  ReleaseTmpReg(reg);
  ReleaseTmpReg(reg2);
  ReleaseTmpReg(Res->Reg);
  free(Res);
  free(name);
  free(name2);
  free(regName);
    
  return code;
}

struct InstrSeq *doAssign(char *name, struct ExprRes *Expr){ 
  struct InstrSeq *code;
  
  if (!FindName(table, name)) {
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  char *Name = (char *)malloc((strlen(name)+5)*sizeof(char));
  strcpy(Name, "var_");
  strcat(Name, name);
  
  code = Expr->Instrs;
  AppendSeq(code, GenInstr(NULL, "sw", TmpRegName(Expr->Reg), Name, NULL));
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
  char *Name = (char *)malloc((strlen(name)+5)*sizeof(char));

  strcpy(Name, "var_");
  strcat(Name, name);
  
  code = Res->Instrs;
  AppendSeq(code, GenInstr(NULL, "sw", TmpRegName(Res->Reg), Name, NULL));
  ReleaseTmpReg(Res->Reg);
  free(Res);
  free(Name);
  
  return code;
}

extern struct InstrSeq *doArrAssign(char *name, struct ExprRes *Res1, struct ExprRes *Res2){
  struct InstrSeq *code;
  char *label = GenLabel();
  int reg = AvailTmpReg();
  int reg2 = AvailTmpReg();

  if (!FindName(table, name)){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Undeclared variable");
  }
  
  char *Name = (char *)malloc((strlen(name) + 5) * sizeof(char));
  strcpy(Name, "var_");
  strcat(Name, name);

  char *name2 = (char *)malloc((strlen(name) +9) * sizeof(char));
  strcpy(name2, "var_len_");
  strcat(name2, name);
  
  AppendSeq(Res1->Instrs, Res2->Instrs);
  code = Res1->Instrs;
  AppendSeq(code, GenInstr(NULL, "bltz", TmpRegName(Res1->Reg), label, NULL));
  AppendSeq(code, GenInstr(NULL, "lw", TmpRegName(reg), name2, NULL));
  AppendSeq(code, GenInstr(NULL, "bge", TmpRegName(Res1->Reg), TmpRegName(reg), label));
  AppendSeq(code, GenInstr(NULL, "lw", TmpRegName(reg), Name, NULL));
  AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg2), "4", NULL));
  AppendSeq(code, GenInstr(NULL, "mul", TmpRegName(reg2), TmpRegName(Res1->Reg), TmpRegName(reg2)));
  AppendSeq(code, GenInstr(NULL, "add", TmpRegName(reg), TmpRegName(reg), TmpRegName(reg2)));

  char *regName = (char *)malloc(6 * sizeof(char));
  strcpy(regName, "(");
  strcat(regName, TmpRegName(reg));
  strcat(regName, ")");
  
  AppendSeq(code, GenInstr(NULL, "sw", TmpRegName(Res2->Reg), regName, NULL));
  AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));

  ReleaseTmpReg(reg);
  ReleaseTmpReg(reg2);
  ReleaseTmpReg(Res1->Reg);
  ReleaseTmpReg(Res2->Reg);
  free(Res1);
  free(Res2);
  free(Name);
  free(name2);
  free(regName);
  
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

extern struct BExprRes *doOR(struct ExprRes *Res1, struct ExprRes *Res2){
  struct BExprRes *bRes;
  char *label = GenLabel();
  char *label2 = GenLabel();
  char *finish = GenLabel();
  int reg1 = AvailTmpReg();

  if (!Res1->isBool || !Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply OR operator to int.");
    exit(0);
  }

  bRes = (struct BExprRes *)malloc(sizeof(struct BExprRes));
  AppendSeq(Res1->Instrs, Res2->Instrs);
  bRes->Instrs = Res1->Instrs;
  bRes->Reg = Res1->Reg;
  bRes->Label = GenLabel();
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(reg1), "1", NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "beq", TmpRegName(Res1->Reg),
				   TmpRegName(reg1), label));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "bne", TmpRegName(Res2->Reg),
				   TmpRegName(reg1), label2));
  AppendSeq(bRes->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(bRes->Reg), "1", NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "j", finish, NULL, NULL));
  AppendSeq(bRes->Instrs, GenInstr(label2, NULL, NULL, NULL, NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(bRes->Reg), "0", NULL));
  AppendSeq(bRes->Instrs, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg1);
  ReleaseTmpReg(Res2->Reg);
  free(Res1);
  free(Res2);
  
  return bRes;
}

extern struct BExprRes *doAND(struct ExprRes *Res1, struct ExprRes *Res2){
  struct BExprRes *bRes;
  char *label = GenLabel();
  char *finish = GenLabel();
  int reg1 = AvailTmpReg();

  if (!Res1->isBool || !Res2->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply AND operator to int.");
    exit(0);
  }

  bRes = (struct BExprRes *)malloc(sizeof(struct BExprRes));
  AppendSeq(Res1->Instrs, Res2->Instrs);
  bRes->Instrs = Res1->Instrs;
  bRes->Reg = Res1->Reg;
  bRes->Label = GenLabel();
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(reg1), "1", NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "bne", TmpRegName(Res1->Reg),
				   TmpRegName(reg1), label));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "bne", TmpRegName(Res2->Reg),
				   TmpRegName(reg1), label));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(bRes->Reg), "1", NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "j", finish, NULL, NULL));
  AppendSeq(bRes->Instrs, GenInstr(label, NULL, NULL, NULL, NULL));
  AppendSeq(bRes->Instrs, GenInstr(NULL, "li", TmpRegName(bRes->Reg), "0", NULL));
  AppendSeq(bRes->Instrs, GenInstr(finish, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg1);
  ReleaseTmpReg(Res2->Reg);
  free(Res1);
  free(Res2);

  return bRes;
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

extern struct InstrSeq *doWhile(struct ExprRes *Res, struct InstrSeq *seq){
  struct InstrSeq *code;
  struct InstrSeq *seq2;
  char *label = GenLabel();
  char *loop = GenLabel();
  int reg = AvailTmpReg();
  int reg2 = AvailTmpReg();

  if (!Res->isBool){
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Type violation: Unable to apply WHILE operator to int.");
    exit(0);
  }

  //seq2 = (struct InstrSeq *)malloc(sizeof(struct InstrSeq));
  //seq2 = Res->Instrs;
  code = GenInstr(loop, NULL, NULL, NULL, NULL);
  AppendSeq(code, Res->Instrs);
  AppendSeq(code, GenInstr(NULL, "li", TmpRegName(reg), "1", NULL));
  AppendSeq(code, GenInstr(NULL, "bne", TmpRegName(Res->Reg),
			   TmpRegName(reg), label));
  AppendSeq(code, seq);
  //AppendSeq(code, Res->Instrs);
  //while(NULL != Res->Instrs){
  //AppendSeq(code, GenInstr(strdup(Res->Instrs->Label), strdup(Res->Instrs->OpCode),
  //			     strdup(Res->Instrs->Oprnd1), strdup(Res->Instrs->Oprnd2),
  //			     strdup(Res->Instrs->Oprnd3)));
  //Res->Instrs = Res->Instrs->Next;
  //}
  //AppendSeq(code, seq2);
  AppendSeq(code, GenInstr(NULL, "j", loop, NULL, NULL));
  AppendSeq(code, GenInstr(label, NULL, NULL, NULL, NULL));
  ReleaseTmpReg(reg);
  ReleaseTmpReg(reg2);
  ReleaseTmpReg(Res->Reg);
  free(Res);
  //  free(seq2);
  
  return code;
}

void doPLFunctDec(char *type, char *Id, struct InstrSeq *Seq){
  struct Funct *function;
  struct SymTab *table;

  table = CreateSymTab(10);
  function = (struct Funct *)malloc(sizeof(struct Funct));
  function->name = strdup(Id);
  function->type = strdup(type);
  function->Instrs = Seq;
  function->table = table;

  addFitem(function);
}

extern struct ExprRes *doPLFunct(char *Id){
  struct ExprRes *Res;

  Res = (struct ExprRes *)malloc(sizeof(struct ExprRes));
  Res->Reg = AvailTmpReg();
  Res->Instrs = GenInstr(NULL, "jal", Id, NULL, NULL);
  AppendSeq(Res->Instrs, GenInstr(NULL, "move", TmpRegName(Res->Reg), "$v0", NULL));
	    
  return Res;
}

extern struct InstrSeq *doReturn(struct ExprRes *Res){
  struct InstrSeq *code;

  code = Res->Instrs;
  AppendSeq(code, GenInstr(NULL, "move", "$v0", TmpRegName(Res->Reg), NULL));
  AppendSeq(code, GenInstr(NULL, "jr", "$ra", NULL, NULL));
	    
  return code;
}

void addFitem(struct Funct *function){  
  struct FunctList *fList2;

  if (NULL == fList->function){
    fList->function = function;
    fList->Next = NULL;
  }
  else{
    fList2 = (struct FunctList *)malloc(sizeof (struct FunctList));
    fList2->function = function;
    fList2->Next = NULL;
    while (NULL != fList->Next){
      fList = fList->Next;
    }
    fList->Next = fList2;
  }
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

extern struct IdList *doIdList(char *Id, struct IdList *List){
  struct IdList *list;

  list = (struct IdList *)malloc(sizeof(struct IdList));
  list->TheEntry = FindName(table, Id);
  list->Next = List;

  return list;
}

extern struct IdList *doArrIdList(char *Id, struct ExprRes *Res, struct IdList *List){
  struct IdList *list;

  list = (struct IdList *)malloc(sizeof(struct IdList));
  list->TheEntry = FindName(table, Id);
  list->Expr = Res;
  list->Next = List;
  
  return list;
}

void printExprList(struct ExprResList *list){
  struct ExprResList *templist;
  struct ExprRes *temp;

  while (templist){
    temp = templist->Expr;
    if (temp->isBool){
      //printf("Variable is type: boolean\n");
    }
    else{
      //printf("Variable is type: int\n");
    }
    templist = templist->Next;
  }
}

extern struct InstrSeq *dupStruct(struct InstrSeq *Instrs){
  struct InstrSeq *seq = (struct InstrSeq *)malloc(sizeof(struct InstrSeq));

  while(NULL != Instrs->Next){
    seq->Label = strdup(Instrs->Label);
    seq->OpCode = strdup(Instrs->OpCode);
    seq->Oprnd1 = strdup(Instrs->Oprnd1);
    seq->Oprnd2 = strdup(Instrs->Oprnd2);
    seq->Oprnd3 = strdup(Instrs->Oprnd3);
    seq->Next = dupStruct(Instrs->Next);

    Instrs = Instrs->Next;
  }
  while (NULL != seq->Next){
    seq = seq->Next;
  }
  seq->Label = strdup(Instrs->Label);
  seq->OpCode = strdup(Instrs->OpCode);
  seq->Oprnd1 = strdup(Instrs->Oprnd1);
  seq->Oprnd2 = strdup(Instrs->Oprnd2);
  seq->Oprnd3 = strdup(Instrs->Oprnd3);
  seq->Next = NULL;
  
  return seq;
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
  //put functions here.
  struct Funct *function;
  while (NULL != fList){
    function = fList->function;
    if (function){
      AppendSeq(code, GenInstr(function->name, NULL, NULL, NULL, NULL));
      AppendSeq(code, function->Instrs);
    }
    fList = fList->Next;
  }
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
    if (0 == strcmp((char *)GetAttr(entry), "string")){
      AppendSeq(code, GenInstr((char *)GetName(entry), ".asciiz", (char *)GetStrVal(entry), NULL, NULL));
    }
    else{
      AppendSeq(code,GenInstr((char *) GetName(entry),".word","0",NULL,NULL));
    }
    entry = NextEntry(table, entry);
  }
  
  WriteSeq(code);
  
  return;
}
