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
  char *string;
  bool boolean;
  struct ExprRes * ExprRes;
  struct InstrSeq * InstrSeq;
  struct BExprRes * BExprRes;
  struct ExprResList *ExprResList;
  struct IdList *IdList;
}

%type <string> Id
%type <string> String
%type <boolean> BVal
 //%type <ExprRes> ArrExp
%type <ExprRes> Number
%type <ExprRes> Factor
%type <ExprRes> Term
%type <ExprRes> Expr
%type <ExprRes> OExpr
%type <ExprRes> AExpr
%type <ExprRes> CExpr
%type <ExprResList> ExprList
%type <InstrSeq> StmtSeq
%type <InstrSeq> Stmt
%type <BExprRes> BFactor
 //%type <BExprRes> BTerm
%type <BExprRes> BExpr
%type <IdList> IdList

%token Ident 		
%token IntLit 	
%token Int
%token Bool
%token TRUE
%token FALSE
%token Write
%token Write_LN
%token Write_SP
%token Write_STR
%token IF
%token ELSE
%token WHILE
%token AND
%token OR
%token EQ
%token NEQ
%token LTE
%token GTE
%token LT
%token GT
%token STR
%token Read
%token RETURN

%%

Prog		:	Declarations StmtSeq						{Finish($2);};
Declarations	:	Dec Declarations						{};
Declarations	:									{};
Dec		:	Int Id ';'	                                                {EnterName(table, $2, &entry);
                                                                                         SetAttr(entry, (void *)"int");};
Dec             :       Bool Id ';'                                                     {EnterName(table, $2, &entry);
                                                                                         SetAttr(entry, (void *)"bool");};
Dec             :       Int Id '(' ')' '{' StmtSeq '}'                                  {doPLFunctDec("int", $2, $6);};
Dec             :       Bool Id '(' ')' '{' StmtSeq '}'                                 {doPLFunctDec("bool", $2, $6);};
StmtSeq 	:	Stmt StmtSeq							{$$ = AppendSeq($1, $2);};
StmtSeq		:									{$$ = NULL;};
Stmt            :       Write_LN ';'                                                    {$$ = doPrintLN();};
Stmt            :       Write '(' ExprList ')' ';'                                      {$$ = doPrintList($3);};
Stmt            :       Read '(' IdList ')' ';'                                         {$$ = doRead($3);};
Stmt            :       Int Id '[' Expr ']' ';'                                         {$$ = enterArr($2, $4, "int[]");};
Stmt            :       Bool Id '[' Expr ']' ';'                                        {$$ = enterArr($2, $4, "bool[]");};
Stmt		:	Write Expr ';'  						{$$ = doPrint($2);};
Stmt            :       Write Id '[' Expr ']' ';'                                       {$$ = doPrintArr($2, $4);};
Stmt            :       Write_SP '(' Expr ')' ';'                                       {$$ = doPrintSP($3);};
Stmt            :       Write_STR '(' String ')' ';'                                    {$$ = doPrintSTR($3);};
Stmt            :       Write_STR '(' Id ')' ';'                                        {$$ = doPrintSTR($3);};
Stmt		:	Id '=' Expr ';'							{$$ = doAssign($1, $3);};
Stmt            :       Id '[' Expr ']' '=' Expr ';'                                    {$$ = doArrAssign($1, $3, $6);};
Stmt		:	IF '(' BExpr ')' '{' StmtSeq '}'				{$$ = doIf($3, $6);};
Stmt            :       IF '(' BExpr ')' '{' StmtSeq '}' ELSE '{' StmtSeq '}'           {$$ = doIfElse($3, $6, $10);};
Stmt            :       WHILE '(' Expr ')' '{' StmtSeq '}'                              {$$ = doWhile($3, $6);};
Stmt            :       RETURN Expr ';'                                                 {$$ = doReturn($2);};
ExprList        :       Expr ',' ExprList                                               {$$ = doList($1, $3);};
ExprList        :       Expr                                                            {$$ = doListItem($1);};
ExprList        :                                                                       {$$ = NULL;};
IdList          :       Id ',' IdList                                                   {$$ = doIdList($1, $3);};
IdList          :       Id '[' Expr ']' ',' IdList                                      {$$ = doArrIdList($1, $3, $6);};
IdList          :       Id '[' Expr ']'                                                 {$$ = doArrIdList($1, $3, NULL);};
IdList          :       Id                                                              {$$ = doIdList($1, NULL);};
IdList          :                                                                       {$$ = NULL;};
Expr            :       Id '(' ')'                                                      {$$ = doPLFunct($1);};
Expr            :       '!' Expr                                                        {$$ = doNOT($2);};
Expr            :       BExpr                                                           {$$ = doConvert($1);};
Expr            :       CExpr                                                           {$$ = $1;};
BExpr           :       BFactor                                                         {$$ = $1;};
BExpr           :       Expr OR OExpr                                                   {$$ = doOR($1, $3);};
Expr            :       OExpr                                                           {$$ = $1;};
BExpr           :       OExpr AND AExpr                                                 {$$ = doAND($1, $3);};
OExpr           :       AExpr                                                           {$$ = $1;};
AExpr           :       '!' AExpr                                                       {$$ = doNOT($2);};
AExpr           :       BFactor                                                         {$$ = doConvert($1);};
BFactor		:	CExpr EQ CExpr							{$$ = doBExpr("seq", $1, $3);};
BFactor         :       CExpr NEQ CExpr                                                 {$$ = doBExpr("sne", $1, $3);};
BFactor         :       CExpr LTE CExpr                                                 {$$ = doBExpr("sle", $1, $3);};
BFactor         :       CExpr GTE CExpr                                                 {$$ = doBExpr("sge", $1, $3);};
BFactor         :       CExpr LT CExpr                                                  {$$ = doBExpr("slt", $1, $3);};
BFactor         :       CExpr GT CExpr                                                  {$$ = doBExpr("sgt", $1, $3);};
BFactor         :       '(' BExpr ')'                                                   {$$ = $2;};
BFactor         :       BVal                                                            {$$ = doBLit($1);};
BVal            :       TRUE                                                            {$$ = true;};
BVal            :       FALSE                                                           {$$ = false;};
CExpr		:	CExpr '+' Term							{$$ = doAdd($1, $3);};
CExpr           :       CExpr '-' Term                                                  {$$ = doSub($1, $3);};
CExpr		:	Term								{$$ = $1;};
Term		:	Term '*' Factor							{$$ = doMult($1, $3);};
Term            :       Term '/' Factor                                                 {$$ = doDiv($1, $3);};
Term            :       Term '%' Factor                                                 {$$ = doMod($1, $3);};
Term		:	Factor                                                          {$$ = $1;};
Factor          :       Factor '^' Number                                               {$$ = doExp($1, $3);};
Factor          :       Number                                                          {$$ = $1;};
Number          :       Id '[' Expr ']'                                                 {$$ = doArrVal($1, $3);};
Number          :       Id                                                              {$$ = doRval($1);};
Number          :       '-' Number                                                      {$$ = doNEG($2);};
Number          :       '(' Expr ')'                                                    {$$ = $2;};
Number          :       IntLit                                                          {$$ = doIntLit(yytext);};
Id		: 	Ident								{$$ = strdup(yytext);};
String          :       STR                                                             {$$ = strdup(yytext);}

%%

int yyerror(char *s)  {
  WriteIndicator(GetCurrentColumn());
  WriteMessage("Illegal Character in YACC");
  return 1;
}
