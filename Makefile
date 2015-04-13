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
	$(RM) *.h~
	$(RM) *.stackdump
	$(RM) *.c~
	$(RM) *.y~
	$(RM) *.l~
	$(RM) *.txt~
