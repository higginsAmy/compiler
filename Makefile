# Amy Higgins
# SymTab.c Makefile
# CS 442
########################

CC= gcc
CFLAGS=-Wall -g

all: comp

comp: Semantics.c main.c SymTab.c SymTab.h IOMngr.c ExprEval.y Semantics.h
	yacc -d ExprEval.y
	flex lex1.l

	$(CC) -o comp y.tab.c lex.yy.c SymTab.c Semantics.c CodeGen.c IOMngr.c main.c $(CFLAGS)

clean:
	$(RM) tests *.o
	$(RM) -rf *.dSYM
	$(RM) Makefile~
	$(RM) SymTab.h~
	$(RM) SymTab.exe.stackdump
	$(RM) tests.exe.stackdump
	$(RM) SymTab.c~
	$(RM) tests.c~
	$(RM) tests.h~
	$(RM) IOMngr.h~
	$(RM) IOMngr.c~
	$(RM) boolExpr.y~
	$(RM) a.exe.stackdump
	$(RM) arithExpr.y~
	$(RM) boolLex.l~
	$(RM) testArith.txt~
	$(RM) arithLex.l~
	$(RM) semantics.c~
	$(RM) semantics.h~
	$(RM) testLex.txt~
	$(RM) lex1.l~
	$(RM) CodeGen.c~
	$(RM) main.c~
	$(RM) ExprEval.y~ 
