%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yaccExample.h"
#include "Semantics.h"
#include "CodeGen.h"

extern int yylex();	/* The next token function. */
extern char *yytext;   /* The matched token text.  */
extern int yyleng;      /* The token text length.   */
extern int yyparse();
void dumpTable();

extern struct SymTab *table;
extern struct SymEntry *entry;

%}


%union {
  long val;
  char * string;
  bool boolean;
  struct ExprRes * ExprRes;
  struct InstrSeq * InstrSeq;
  struct BExprRes * BExprRes;
  struct ExprResList *ExprResList;
}

%type <string> Id
%type <boolean> BVal
%type <ExprRes> Number
%type <ExprRes> Factor
%type <ExprRes> Term
%type <ExprRes> Expr
%type <ExprResList> ExprList
%type <InstrSeq> StmtSeq
%type <InstrSeq> Stmt
%type <BExprRes> BFactor
%type <BExprRes> BTerm
%type <BExprRes> BExpr

%token Ident 		
%token IntLit 	
%token Int
%token Bool
%token TRUE
%token FALSE
%token Write
%token IF
%token AND
%token OR
%token EQ
%token NEQ
%token LTE
%token GTE
%token LT
%token GT

%%

Prog		:	Declarations StmtSeq						{Finish($2);};
Declarations	:	Dec Declarations						{};
Declarations	:									{};
Dec		:	Int Id ';'	                                                {EnterName(table, $2, &entry);
                                                                                         SetAttr(entry, (void *)"int");};
Dec             :       Bool Id ';'                                                     {EnterName(table, $2, &entry);
                                                                                         SetAttr(entry, (void *)"bool");};
StmtSeq 	:	Stmt StmtSeq							{$$ = AppendSeq($1, $2);} ;
StmtSeq		:									{$$ = NULL;};
Stmt		:	Write Expr ';'							{$$ = doPrint($2);};
Stmt            :       Write '(' ExprList ')' ';'                                      {$$ = doPrintList($3);};
Stmt		:	Id '=' Expr ';'							{$$ = doAssign($1, $3);};
Stmt            :       Id '=' BExpr ';'                                                {$$ = doBAssign($1, $3);};
Stmt		:	IF '(' BExpr ')' '{' StmtSeq '}'				{$$ = doIf($3, $6);};
ExprList        :       Expr ',' ExprList                                               {$$ = doList($1, $3);};
ExprList        :       BExpr ',' ExprList                                              {$$ = doBList($1, $3);};
ExprList        :       Expr                                                            {$$ = doListItem($1);};
ExprList        :       BExpr                                                           {$$ = doBListItem($1);};
ExprList        :                                                                       {$$ = NULL;};
BExpr           :       BExpr OR BTerm                                                  {$$ = doOR($1, $3);};
BExpr           :       BTerm                                                           {$$ = $1;};
BTerm           :       BTerm AND BFactor                                               {$$ = doAND($1, $3);};
BTerm           :       BFactor                                                         {$$ = $1;};
BFactor         :       '!' BFactor                                                     {$$ = doNOT($2);};
BFactor		:	Expr EQ Expr							{$$ = doBExpr("seq", $1, $3);};
BFactor         :       Expr NEQ Expr                                                   {$$ = doBExpr("sne", $1, $3);};
BFactor         :       Expr LTE Expr                                                   {$$ = doBExpr("sle", $1, $3);};
BFactor         :       Expr GTE Expr                                                   {$$ = doBExpr("sge", $1, $3);};
BFactor         :       Expr LT Expr                                                    {$$ = doBExpr("slt", $1, $3);};
BFactor         :       Expr GT Expr                                                    {$$ = doBExpr("sgt", $1, $3);};
BFactor         :       Id                                                              {$$ = doBval($1);};
BFactor         :       '(' BExpr ')'                                                   {$$ = $2;};
BFactor         :       BVal                                                            {$$ = doBLit($1);};
BVal            :       TRUE                                                            {$$ = true;};
BVal            :       FALSE                                                           {$$ = false;};
Expr		:	Expr '+' Term							{$$ = doAdd($1, $3);};
Expr            :       Expr '-' Term                                                   {$$ = doSub($1, $3);};
Expr		:	Term								{$$ = $1;};
Term		:	Term '*' Factor							{$$ = doMult($1, $3);};
Term            :       Term '/' Factor                                                 {$$ = doDiv($1, $3);};
Term            :       Term '%' Factor                                                 {$$ = doMod($1, $3);};
Term		:	Factor                                                          {$$ = $1;};
Factor          :       Factor '^' Number                                               {$$ = doExp($1, $3);};
Factor          :       Number                                                          {$$ = $1;};
Number          :       Id                                                              {$$ = doRval($1);};
Number          :       '-' Number                                                      {$$ = doNEG($2);};
Number          :       '(' Expr ')'                                                    {$$ = $2;};
Number          :       IntLit                                                          {$$ = doIntLit(yytext);};
Id		: 	Ident								{$$ = strdup(yytext);}
 
%%

int yyerror(char *s)  {
  WriteIndicator(GetCurrentColumn());
  WriteMessage("Illegal Character in YACC");
  return 1;
}
