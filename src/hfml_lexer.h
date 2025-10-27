#ifndef HFML_LEXER_H
#define HFML_LEXER_H

enum tokens {
    OPEN_COMP = 0,
    CLOSE_COMP,
    OPEN_BR,
    CLOSE_BR,
    OPEN_PAREN,
    CLOSE_PAREN,
    COLON,
    COMMA,
    EQUALS,
    HASH,
    IDENTIFIER,
    NUMBER,
    LITERAL,
    WHITESPACE,
    HFML_TOKEN_COUNT
};

#include "splicestr.h"

const char* int_to_token(int t);
void hfml_lexer_compile_rules(struct ruleset *rs);


#endif