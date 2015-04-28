#include <string.h>
#include "IOMngr.h"

FILE *src = NULL;
FILE *lst = NULL;
char buffer[MAXLINE];
bool wrote_line;
int line;
int col;

bool OpenFiles(const char *ASourceName, const char *AListingName){
  line = 0;
  col = -1;
  wrote_line = false;
  
  src = fopen(ASourceName, "r");
  if (0 == src){
    printf("Could not open source file!\n");
    return false;
  }
  if (NULL != AListingName){
    lst = fopen(AListingName, "w");
    if (0 == lst){
      printf("Could not open listing file!\n");
      return false;
    }
  }
  return true;
}

void CloseFiles(){
  if (NULL != src){
    if (0 != fclose(src)){
      printf("Error closing source file!\n");
    }
  }
  if (NULL != lst){
    if (0 != fclose(lst)){
      printf("Error closing listing file!\n");
    }
  }
}

char GetSourceChar(){
  char c;
  char ln[MAXLINE];

  col ++;
  c = buffer[col];
  if (('\n' != c) && (EOF != c) && ('\0' != c) && (0 != line)){
    return c;
  }
  if (NULL != fgets(buffer, MAXLINE, src)){
    //printf("LINE: %s\n", ln);
    if (!wrote_line && NULL != lst && 0 != line){
      fprintf(lst, "%d. %s", line, ln);
    }
    strcpy(ln, buffer);
    line ++;
    col = 0;
    wrote_line = false;
    return buffer[col];
  }
  else {
    if (!wrote_line && NULL != lst){
      fprintf(lst, "%d. %s", line, ln);
    }
    //printf("Reached end of file!\n");
    return EOF;
  }
}

void WriteIndicator(int AColumn){
  int i;
  char space = ' ';
  char carat = '^';

  if (!wrote_line){
    wrote_line = true;
  }
  if (NULL == lst){
    printf("%d. %s\n", line, buffer);
    for (i = 0; i < AColumn; i++){
      printf("%c", space);
    }
    printf("%c\n", carat);
  }
  else {
    fprintf(lst, "%d. %s\n", line, buffer);
    for (i = 0; i < AColumn; i++){
      if (EOF == fputc((int)space, lst)){
	printf("Unable to add character to file.\n");
      }
    }
    fprintf(lst, "%c\n", carat);
  }
}

void WriteMessage(const char *AMessage){
  if (NULL == lst){
    printf("%s\n", AMessage);
  }
  else {
    fprintf(lst, "%s\n", AMessage);
  }
}

int GetCurrentLine(){
  return line;
}

int GetCurrentColumn(){
  return col;
}
