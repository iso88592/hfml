%{
    #include "hfml.tab.h"
    #include <stdio.h>
%}

%option yylineno

identifier           [a-zA-Z_][a-zA-Z0-9_$]*
literal              [a-zA-Z_ 0-9$!]+
whitespace           [ \t\r\n]*
number               [0-9]+

%%
"<"                  return OPEN_STR;
">"                  return CLOSE_STR;
"{"                  return OPEN_CBR;
"}"                  return CLOSE_CBR;
"["                  return OPEN_BR;
"]"                  return CLOSE_BR;
"("                  return OPEN_PAREN;
")"                  return CLOSE_PAREN;
":"                  return COLON;
","                  return COMMA;
"="                  return EQUALS;
{identifier}         { yylval.str = strdup(yytext); return IDENTIFIER; }
{number}             { yylval.num = atoi(yytext); return NUMBER; }
{literal}            { yylval.str = strdup(yytext); return LITERAL; }
{whitespace}         /*ignore*/

%%

int yywrap() {
    return 1;
}