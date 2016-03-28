%{
	#include"bison.tab.h"
	#include"global.h"
	#define YYSTYPE char*
%}
char [^ \t\n\>\<&\|]
space [ \t]
string {char}+

%%

\n {return END;};
{space} {strcat(inputBuff,yytext);};
\< {strcat(inputBuff,yytext);return '<';};
\> {strcat(inputBuff,yytext);return '>';};
\| {isPipe=1;return '|';};
& {strcat(inputBuff,yytext);return '&';};
{string} {yylval = strdup(yytext); strcat(inputBuff,yytext);return STRING;};
%%
int yywrap(){
return 1;
}
