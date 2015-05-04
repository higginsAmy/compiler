#include <stdio.h>
#include "Semantics.h"
#include "CodeGen.h"
#include "SymTab.h"
#include "IOMngr.h"

extern int yyparse();

struct SymTab *table;
struct SymTab *ProcSymTab;
struct SymEntry *entry;
struct FunctList *fList;
int inProc =0;
FILE *aFile;

int main(int argc, char * argv[]) {
  //printf("\n");
  table = CreateSymTab(33);
  fList = (struct FunctList *)malloc(sizeof(struct FunctList));
  //assumes there is a listing file
  OpenFiles(argv[1], argv[2]);
  if (argc == 4){ 
    aFile = fopen(argv[3], "w");
  }
  else{
    aFile = stdout;
  }

  yyparse();
}

