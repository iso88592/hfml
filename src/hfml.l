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

ASC1    [\x20-\x22]
ASC2    [\x24-\x39]
ASC3    [\x3b]
ASC4    [\x3f-\x5a]
ASC5    [\x5e-\x7f]
ASC     {ASC1}|{ASC2}|{ASC3}|{ASC4}|{ASC5}
U       [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]
UANY    {ASC}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}

identifier           [a-zA-Z_][a-zA-Z0-9_$]*
literal              {UANY}
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
"#"                  return HASH;
{identifier}         { yylval->str = yytext; return IDENTIFIER; }
{number}             { yylval->num = atoi(yytext); return NUMBER; }
{literal}            { yylval->str = yytext; return LITERAL; }
{whitespace}         /*ignore*/

%%

int yywrap(void*) {
    return 1;
}