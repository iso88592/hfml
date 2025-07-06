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
    void report_error(void* p, const char* str);
    void yylex_init_extra(void*, void*);
    void yyset_debug(int, void*);
    void* yy_scan_string(const char*, void*);
    void yyset_lineno(int, void*);
    void yylex_destroy(void*);
    #define EXTRA (yyget_extra(scanner))
}
%{
#include "hfml.tab.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct yy_buffer_state * YY_BUFFER_STATE;

extern void restore_phase();
extern void reset_phase();
extern void create_tag(void* p);
extern void pop_tag(void* p);
extern void append_attribute(void* p, const char* str);
extern void append_attribute_id(void* p, const char* str);
extern void append_literal(void* p, const char* str);
extern void append_modifier_number(void* p, int z);
extern void add_event(void* p);
extern void create_list(void* p);
extern void append_list(void* p, const char* item);
extern void append_list_id(void* p, const char* item);
extern void store_name(void* p, const char* str);
extern void store_event_handler(void* p, const char* str);
extern const char* get_error_context(void* p, int line, int col);
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
%token OPEN_COMP CLOSE_COMP OPEN_CBR CLOSE_CBR OPEN_BR CLOSE_BR OPEN_PAREN CLOSE_PAREN COLON COMMA EQUALS HASH OPEN_STR CLOSE_STR
%type <mystr> string

%start start

%%

start: components

string: LITERAL { $$ = mystr_construct_s($1); }

components : components component
           |

component : OPEN_COMP { create_tag(EXTRA->caller); } str_internals { pop_tag(EXTRA->caller); } CLOSE_COMP
str_internals : str_internals str_internal
              |

str_internal : attribute 
             | component
             | string { 
                    char* p = mystr_to_c($1);
                    append_literal(EXTRA->caller, p); 
                    free(p);
                    mystr_destroy($1); 
                }

attribute : OPEN_BR attribute_selector attribute_modifiers CLOSE_BR

attribute_modifiers : attribute_modifiers COLON  attribute_modifier
                    |

attribute_selector : IDENTIFIER { append_attribute(EXTRA->caller, $1); }
                   | HASH IDENTIFIER { append_attribute_id(EXTRA->caller, $2); }

attribute_modifier : event
                   | NUMBER { append_modifier_number(EXTRA->caller, $1); }

event : IDENTIFIER { store_name(EXTRA->caller, $1); } EQUALS params { add_event(EXTRA->caller); }

params : IDENTIFIER { store_event_handler(EXTRA->caller, $1); } OPEN_PAREN { create_list(EXTRA->caller); } param_list CLOSE_PAREN 
       |

param_list : param
           | param_list COMMA param

param : string { 
        char* p = mystr_to_c($1);
        append_list(EXTRA->caller, p); 
        free(p);
        mystr_destroy($1); 
    }
      | HASH IDENTIFIER {
        append_list_id(EXTRA->caller, $2);
      }

%%
void yyerror(YYLTYPE* yyllocp, yyscan_t scanner, const char* s) {
    char buffer[1024];
    char* line = get_error_context(EXTRA->caller, yyllocp->first_line, yyllocp->first_column);
    snprintf(buffer, 1023, "Error: %s at line %d:%d near `%s'", s, yyllocp->first_line, yyllocp->first_column , line);
    free(line);
    report_error(EXTRA->caller, buffer);
}

typedef struct {
    yyscan_t scanner;
    ScannerExtraData extra;
    bool in_use;

} ScannerWrapper;

ScannerWrapper** scanners;
int scannerSpin = 0;

#ifndef SCANNER_COUNT
#define SCANNER_COUNT 64
#endif

__attribute__((constructor))
void create_scanners() {
    scanners = malloc(sizeof(ScannerWrapper*) * SCANNER_COUNT);
    for (int i = 0; i < SCANNER_COUNT; i++) {
        scanners[i] = malloc(sizeof(ScannerWrapper));
        bzero(scanners[i], sizeof(ScannerWrapper));
        yylex_init_extra(&scanners[i]->extra, &scanners[i]->scanner);
#ifdef YY_DEBUG    
        yyset_debug(1, scanners[i]->scanner);
#else
        yyset_debug(0, scanners[i]->scanner);
#endif    
        scanners[i]->in_use = false;
        fprintf(stderr, "Created scanner %d at %p with scanner %p extra data at %p\n", i, scanners[i], scanners[i]->scanner, &scanners[i]->extra);
    }
    reset_phase();
}

__attribute__((destructor))
void destroy_scanners() {
    restore_phase();
    for (int i = 0; i < SCANNER_COUNT; i++) {
        yylex_destroy(scanners[i]->scanner);
        free(scanners[i]);
    }
    free(scanners);
}

ScannerWrapper* acquire_scanner() {
    ScannerWrapper* result;
    do {
        result = scanners[scannerSpin];
        scannerSpin++;
        scannerSpin %= SCANNER_COUNT;
    } while (result->in_use == true);
    result->in_use = true;
    return result;
}

void free_scanner(ScannerWrapper* wrapper) {
    wrapper->in_use = false;
}

int parse_str(const char* str, void* p) {
    ScannerWrapper* wrapper = acquire_scanner();
    wrapper->extra.caller = p;
    YY_BUFFER_STATE buffer = yy_scan_string(str, wrapper->scanner);
    yyset_lineno(1, wrapper->scanner);
    int result = yyparse(wrapper->scanner);
    free_scanner(wrapper);
    return result;
}
