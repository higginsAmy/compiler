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
  struct ExprRes * ExprRes;
  struct InstrSeq * InstrSeq;
  struct BExprRes * BExprRes;
}

%type <string> Id
%type <ExprRes> Number
%type <ExprRes> Factor
%type <ExprRes> Term
%type <ExprRes> Expr
%type <InstrSeq> StmtSeq
%type <InstrSeq> Stmt
%type <BExprRes> BExpr

%token Ident 		
%token IntLit 	
%token Int
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
Dec		:	Int Ident {EnterName(table, yytext, &entry); }';'	        {};
StmtSeq 	:	Stmt StmtSeq							{$$ = AppendSeq($1, $2);} ;
StmtSeq		:									{$$ = NULL;};
Stmt		:	Write Expr ';'							{$$ = doPrint($2);};
Stmt		:	Id '=' Expr ';'							{$$ = doAssign($1, $3);};
Stmt		:	IF '(' BExpr ')' '{' StmtSeq '}'				{$$ = doIf($3, $6);};
BExpr           :       BExpr AND BExpr                                                 {$$ = doAND($1, $3);};
BExpr           :       BExpr OR BExpr                                                  {$$ = doOR($1, $3);};
BExpr		:	Expr EQ Expr							{$$ = doINEQ("seq", $1, $3);};
BExpr           :       Expr NEQ Expr                                                   {$$ = doINEQ("sne", $1, $3);};
BExpr           :       Expr LTE Expr                                                   {$$ = doINEQ("sle", $1, $3);};
BExpr           :       Expr GTE Expr                                                   {$$ = doINEQ("sge", $1, $3);};
BExpr           :       Expr LT Expr                                                    {$$ = doINEQ("slt", $1, $3);};
BExpr           :       Expr GT Expr                                                    {$$ = doINEQ("sgt", $1, $3);};
BExpr           :       '(' BExpr ')'                                                   {$$ = $2;};
Expr		:	Expr '+' Term							{$$ = doAdd($1, $3);};
Expr            :       Expr '-' Term                                                   {$$ = doSub($1, $3);};
Expr		:	Term								{$$ = $1;};
Term		:	Term '*' Factor							{$$ = doMult($1, $3);};
Term            :       Term '/' Factor                                                 {$$ = doDiv($1, $3);};
Term            :       Term '%' Factor                                                 {$$ = doMod($1, $3);};
Term		:	Factor                                                          {$$ = $1;};
Factor          :       Factor '^' Number                                               {$$ = doExp($1, $3);};
Factor          :       Number                                                          {$$ = $1;};
Number          :       '-' Number                                                      {$$ = doNEG($2);};
Number          :       '(' Number ')'                                                  {$$ = $2;};
Number		:	IntLit								{$$ = doIntLit(yytext);};
Number		:	Ident								{$$ = doRval(yytext);};
Id		: 	Ident								{$$ = strdup(yytext);}
 
%%

int yyerror(char *s)  {
  WriteIndicator(GetCurrentColumn());
  WriteMessage("Illegal Character in YACC");
  return 1;
}
