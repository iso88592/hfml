%define api.pure
%locations
%param { yyscan_t scanner }
%code requires {
    #include "mystr.h"
    #include "scanner_extra.h"
    typedef void* yyscan_t;
}
%code {
    int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
    void yyerror(YYLTYPE* yyllocp, yyscan_t scanner, const char* msg);
    #define EXTRA (yyget_extra(scanner))
}
%{
#include "hfml.tab.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern void create_tag(void* p);
extern void append_attribute(void* p, const char* str);
extern void append_literal(void* p, const char* str);
extern char* myitoa(int i);
extern void yyset_extra ( YY_EXTRA_TYPE user_defined , yyscan_t yyscanner );
extern YY_EXTRA_TYPE yyget_extra ( yyscan_t yyscanner );
extern char* concat(char* c, char* str);
%}


%union {
    char* str;
    int num;
    struct mystr* mystr;
}

%token <str> IDENTIFIER
%token <str> LITERAL
%token <num> NUMBER
%token OPEN_STR CLOSE_STR OPEN_CBR CLOSE_CBR OPEN_BR CLOSE_BR OPEN_PAREN CLOSE_PAREN COLON COMMA EQUALS HASH
%type <mystr> string string_mul

%start start

%%

start: strlist
strlist : str strlist 
        |

string: LITERAL { $$ = mystr_construct_s($1); }
      | HASH { $$ = mystr_construct_s("#"); }
      | COLON { $$ = mystr_construct_s(":"); }
      | IDENTIFIER { $$ = mystr_construct_s($1); }
      | NUMBER { $$ = mystr_construct_s(myitoa($1)); }

str : OPEN_STR str_internals CLOSE_STR { create_tag(EXTRA->caller); }
str_internals : str_internal str_internals 
              | { }

str_internal : attribute 
             | string { 
                char* p = mystr_to_c($1); 
                append_literal(EXTRA->caller, p); 
                free(p);
                mystr_destroy($1); 
                }
             | str

attribute : OPEN_BR attribute_selector attribute_modifiers CLOSE_BR

attribute_modifiers : COLON attribute_modifier attribute_modifiers
                    |

attribute_selector : IDENTIFIER { append_attribute(EXTRA->caller, $1); }
                   | HASH IDENTIFIER { append_literal(EXTRA->caller, $2); }

attribute_modifier : event
                   | NUMBER

event : IDENTIFIER params {  }
params : OPEN_PAREN str_comma CLOSE_PAREN 
       | EQUALS str
       |

str_comma : str 
          | str COMMA str_comma 

%%
void yyerror(YYLTYPE* yyllocp, yyscan_t scanner, const char* s) {
    char buffer[1024];
    snprintf(buffer, 1023, "Error: %s at line %d:%d near `%s'", s, yyllocp->first_line, yyllocp->first_column , "??");
    report_error(EXTRA->caller, buffer);
}

int parse_str(const char* str, void* p) {
    yyscan_t scanner;
    ScannerExtraData extra;
    yylex_init_extra(&extra, &scanner);
    extra.caller = p;
    yyset_debug(1, scanner);
    YY_BUFFER_STATE msb = yy_scan_string(str, scanner);
    yyset_lineno(1, scanner);
    int result = yyparse(scanner);
    yylex_destroy(scanner);

    return result;
}
