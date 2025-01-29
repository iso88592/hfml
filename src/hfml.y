%{
#include "hfml.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yylex();
extern void yyerror(const char* s);
extern int yylineno;
extern char* yytext;
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern void create_tag();
extern void append_attribute(const char* str);
extern void append_literal(const char* str);
extern char* myitoa(int i);
char* lastItem;
char* attr;
%}

%union {
    char* str;
    int num;
}

%token <str> IDENTIFIER
%token <str> LITERAL
%token <num> NUMBER
%token OPEN_STR CLOSE_STR OPEN_CBR CLOSE_CBR OPEN_BR CLOSE_BR OPEN_PAREN CLOSE_PAREN COLON COMMA EQUALS

%start start

%%

start: strlist
strlist : str strlist 
        |

str : OPEN_STR str_internals CLOSE_STR { create_tag(); }
str_internals : str_internal str_internals 
              | { }

str_internal : attribute 
             | LITERAL { append_literal($1); }
             | IDENTIFIER { append_literal($1); }
             | NUMBER { append_literal(myitoa($1)); }
             | str

attribute : OPEN_BR attribute_selector attribute_modifiers CLOSE_BR

attribute_modifiers : COLON attribute_modifier attribute_modifiers
                    |

attribute_selector : IDENTIFIER { append_attribute($1); }

attribute_modifier : event
                   | NUMBER

event : IDENTIFIER params
params : OPEN_PAREN str_comma CLOSE_PAREN 
       | EQUALS str
       |

str_comma : str 
          | str COMMA str_comma 

%%
void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d near `%s'\n", s, yylineno, yytext);
}

int parse_str(const char* str) {
    YY_BUFFER_STATE msb = yy_scan_string(str);
    yylineno = 1;
    int result = yyparse();
    yy_delete_buffer(msb);
    return result;
}
