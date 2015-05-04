%{
#include "yaccExample.h"
#include "y.tab.h"

#define YY_INPUT(buf,result,max_size) \
	{ int c = GetSourceChar(); \
          result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
        }

%}


letter [A-Za-z]
digit [0-9]

%%
if					{return IF;}
int					{return Int;}
bool                                    {return Bool;}
true                                    {return TRUE;}
false                                   {return FALSE;}
else                                    {return ELSE;}
while                                   {return WHILE;}
read                                    {return Read;}
printstr                                {return Write_STR;}
println                                 {return Write_LN;}
printsp                                 {return Write_SP;}
print					{return Write;}
{letter}({letter}|{digit})*	        {return Ident;}
{digit}{digit}*		                {return IntLit;}
\&\&                                    {return AND;}
\|\|                                    {return OR;}
\=\=					{return EQ;}
\!\=                                    {return NEQ;}
\<\=                                    {return LTE;}
\>\=                                    {return GTE;}
\<                                      {return LT;}
\>                                      {return GT;}
\=					{return '=';}
\!                                      {return '!';}
\+					{return '+';}
\-                                      {return '-';}
\*					{return '*';}
\/                                      {return '/';}
\%                                      {return '%';}
\^                                      {return '^';}
\,                                      {return ',';}
\;					{return ';';}
\{					{return '{';}
\}					{return '}';}
\(					{return '(';}
\)    					{return ')';}
\[                                      {return '[';}
\]                                      {return ']';}

[ ]				        {}
\t				        {}
\r				        {}
\n				        {}

\".*\"                                  {return STR;}
.				        {WriteIndicator(GetCurrentColumn());
                                         WriteMessage("Illegal Character in Lex.");}

%%

	
int yywrap () {
   return 1;
}
			
