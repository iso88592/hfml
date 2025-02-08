%option reentrant bison-bridge bison-locations
%option noyyalloc noyyrealloc noyyfree

%{
    #include "hfml.tab.h"
    #include "scanner_extra.h"
    #include <stdio.h>
    #include <string.h>

#define YY_USER_ACTION                                             \
  yylloc->first_line = yylloc->last_line;                          \
  yylloc->first_column = yylloc->last_column;                      \
  if (yylloc->last_line == yylineno)                               \
    yylloc->last_column += yyleng;                                 \
  else {                                                           \
    yylloc->last_line = yylineno;                                  \
    yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n'); \
  }
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
{identifier}         { yylval->str = yytext; return IDENTIFIER; }
{number}             { yylval->num = atoi(yytext); return NUMBER; }
{literal}            { yylval->str = yytext; return LITERAL; }
{whitespace}         /*ignore*/

%%

int yywrap(void*) {
    return 1;
}