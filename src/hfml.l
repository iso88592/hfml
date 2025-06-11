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
%option nounput noinput

ASC     [\x20-\x7a\x7c\x7e-\x7f]
U       [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]
UANY    [\n]|{ASC}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}
UESC    {UANY}|"\\{"|"\\}"

identifier           [a-zA-Z_][a-zA-Z0-9_$]*
literal              "{"{UESC}+"}"
whitespace           [ \t\r\n]*
number               [0-9]+

%%
"<"                  return OPEN_COMP;
">"                  return CLOSE_COMP;
"["                  return OPEN_BR;
"]"                  return CLOSE_BR;
"("                  return OPEN_PAREN;
")"                  return CLOSE_PAREN;
":"                  return COLON;
","                  return COMMA;
"="                  return EQUALS;
"#"                  return HASH;
{identifier}         { yylval->str = yytext; return IDENTIFIER; }
{number}             { yylval->num = atoi(yytext); return NUMBER; }
{literal}            { yylval->str = yytext; return LITERAL; }
"{"                  return OPEN_STR;
"}"                  return CLOSE_STR;
{whitespace}         /*ignore*/

%%

int yywrap(void*) {
    return 1;
}