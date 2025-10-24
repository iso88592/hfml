#include <regex.h>
#include <stdlib.h>
#include "hfml_lexer.h"

const char* hfml_lut[HFML_TOKEN_COUNT] = {"OPEN_COMP", "CLOSE_COMP", "OPEN_BR", "CLOSE_BR", "OPEN_PAREN", "CLOSE_PAREN", "COLON", "COMMA", "EQUALS",
    "HASH", "IDENTIFIER", "NUMBER", "LITERAL", "WHITESPACE"};

const char* int_to_token(int t) {
    if (t < HFML_TOKEN_COUNT) {
        return hfml_lut[t];
    }
    return "UNDEFINED";
}


void hfml_lexer_compile_rules(struct ruleset *rs) {

    rs->rules[OPEN_COMP] = "<";
    rs->rules[CLOSE_COMP] = ">";
    rs->rules[OPEN_BR] = "\\[";
    rs->rules[CLOSE_BR] = "\\]";
    rs->rules[OPEN_PAREN] = "\\(";
    rs->rules[CLOSE_PAREN] = "\\)";
    rs->rules[COLON] = ":";
    rs->rules[COMMA] = ",";
    rs->rules[EQUALS] = "=";
    rs->rules[HASH] = "#";
    rs->rules[IDENTIFIER] = "[a-zA-Z_][a-zA-Z0-9_$]*";
    rs->rules[NUMBER] = "[0-9]+";
    rs->rules[LITERAL] = "\\{[^{}]*\\}";
    rs->rules[WHITESPACE] = "[ \t\n\r]+";
    rs->length = 14;
}