#include "SymTab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SymTab *CreateSymTab(int	Size){
  struct SymTab *table;
  int i;

  table = (struct SymTab *)malloc(sizeof(struct SymTab));
  table->Size = Size;
  table->Contents = (struct SymEntry **)malloc(Size * sizeof(struct SymEntry *));
  for (i = 0; i < Size; i++){
    table->Contents[i] = NULL;
  }

  return table;
}

void DestroySymTab(struct SymTab *ATable){
  struct SymEntry *entry = FirstEntry(ATable);
  struct SymEntry *next;
  int i;

  //printf("Freeing entries\n");
  while (NULL != entry){
    next = NextEntry(ATable, entry);
    //printf("Freeing %s\n",entry->Name);
    free(entry->Name);
    free(entry);
    entry = next;
  }
  for (i = 0; i < ATable->Size; i++){
    ATable->Contents[i] = NULL;
  }
  free(ATable->Contents);
  ATable->Contents = NULL;
  free(ATable);
}

bool EnterName(struct SymTab *ATable, const char *Name,	struct SymEntry	**AnEntry){
  struct SymEntry *temp;
  struct SymEntry *new_entry;
  char *name;
  int hash;

  if (NULL != FindName(ATable, Name)){
    *AnEntry = FindName(ATable, Name);
    return false;
  }
  hash = hash_code(Name, ATable->Size);
  //printf("Hashed name %s to location %d.\n", Name, hash);
  temp = ATable->Contents[hash];
  new_entry = (struct SymEntry *)malloc(sizeof(struct SymEntry));
  name = strdup(Name);
  new_entry->Name = name;
  new_entry->Next = NULL;
  new_entry->Attributes = NULL;
  if (NULL != temp){
    while (NULL != temp->Next){
      temp = temp->Next;
    }
    temp->Next = new_entry;
  }
  else{
    ATable->Contents[hash] = new_entry;
  }
  *AnEntry = new_entry;

  return true;
}

struct SymEntry	*FindName(struct SymTab	*ATable, const char *Name){
  struct SymEntry *entry = FirstEntry(ATable);
  struct SymEntry *next;

  while (NULL != entry){
    //printf("Entry isn't null.\n");
    next = NextEntry(ATable, entry);
    //printf("String comparison\n");
    if (0 == strcmp(entry->Name, Name)){
      //printf("Duplicate Name Found!\n");
      return entry;
    }
    //printf("Advance to the next entry.\n");
    entry = next;
  }
  
  return entry;
}

void SetAttr(struct SymEntry *AnEntry, void *Attributes){
  //printf("Setting Attributes of %s to %d\n", AnEntry->Name, Attributes);
  AnEntry->Attributes = Attributes;
}

void *GetAttr(struct SymEntry *AnEntry){
  //printf("Getting Attributes of %s- %d\n", AnEntry->Name, AnEntry->Attributes);
  return AnEntry->Attributes;
}

const char *GetName(struct SymEntry *AnEntry){

  return AnEntry->Name;
}

struct SymEntry	*FirstEntry(struct SymTab *ATable){
  struct SymEntry *temp;
  int i;
  
  temp = ATable->Contents[0];
  if (NULL == temp){
    for (i = 1; i < ATable->Size; i++){
      temp = ATable->Contents[i];
      if (NULL != temp){
	break;
      }
    }
  }
  
  return temp;
}

struct SymEntry	*NextEntry(struct SymTab *ATable, struct SymEntry *AnEntry){
  struct SymEntry *temp;
  int idx = hash_code(AnEntry->Name, ATable->Size);
  int i;

  temp = AnEntry->Next;
  if (NULL != temp){
    return temp;
  }
  for (i = idx+1; i < ATable->Size; i++){
    temp = ATable->Contents[i];
    if (NULL != temp){
      break;
    }
  }
  
  return temp;
}

int hash_code(const char *Name, int Size){
  int temp;
  int total = 0;
  int i;

  for (i = 0; i < strlen(Name); i++){
    temp = Name[i];
    total += temp;
  }
  total = abs(total%Size);

  return total;
}
