#include <regex.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hfml_lexer.h"

const char* hfml_lut[HFML_TOKEN_COUNT] = {"OPEN_COMP", "CLOSE_COMP", "OPEN_BR", "CLOSE_BR", "OPEN_PAREN", "CLOSE_PAREN", "COLON", "COMMA", "EQUALS",
    "HASH", "IDENTIFIER", "NUMBER", "LITERAL", "WHITESPACE"};

const char* int_to_token(int t) {
    if (t < HFML_TOKEN_COUNT) {
        return hfml_lut[t];
    }
    return "UNDEFINED";
}

bool is_small_letter(char a) {
    return (a>='a') && (a<='z');
}
bool is_capital_letter(char a) {
    return (a>='A') && (a<='Z');
}

bool is_decimal(char a) {
    return (a>='0') && (a<='9');

}

bool is_letter(char a) {
    return is_small_letter(a) || is_capital_letter(a);
}

bool is_space(char a) {
    return (a == ' ' || a == '\r' || a == '\t' || a == '\n');
}

#define CASE(tchr, ttype) case tchr: {    \
            matches[0].rm_so = 0;         \
            matches[0].rm_eo = 1;         \
            matches[1 + ttype].rm_so = 0; \
            matches[1 + ttype].rm_eo = 1; \
            return 0;                     \
        }

int quick_parse(const char* data, regmatch_t matches[]) {
    switch (data[0])
    {
        CASE('<', OPEN_COMP)
        CASE('>', CLOSE_COMP)
        CASE('[', OPEN_BR)
        CASE(']', CLOSE_BR)
        CASE('(', OPEN_PAREN)
        CASE(')', CLOSE_PAREN)
        CASE(':', COLON)
        CASE(',', COMMA)
        CASE('=', EQUALS)
        CASE('#', HASH)
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_': {
            int i = 0;
            while (data[++i] != 0) {
                if ((!is_letter(data[i])) && 
                    (!is_decimal(data[i])) && 
                    (data[i] != '_') && 
                    (data[i] != '$')) break;
            }
            matches[0].rm_eo = i;
            matches[0].rm_so = 0;
            matches[1 + IDENTIFIER].rm_eo = i;
            matches[1 + IDENTIFIER].rm_so = 0;

            return 0;
        }
        case '0' ... '9': {
            int i = 0;
            while (data[++i] != 0) {
                if (!is_decimal(data[i])) break;
            }
            matches[0].rm_eo = i;
            matches[0].rm_so = 0;
            matches[1 + NUMBER].rm_eo = i;
            matches[1 + NUMBER].rm_so = 0;
            return 0;
        }
        case '{': {
            int i = 0;
            while (data[++i] != 0) {
                if (data[i] == '{') return 1;
                if (data[i] == '}') break;
            }
            if (data[i] == 0) return 1;
            matches[0].rm_eo = i+1;
            matches[0].rm_so = 0;
            matches[1 + LITERAL].rm_eo = i+1;
            matches[1 + LITERAL].rm_so = 0;
            return 0;

        }
        case ' ':
        case '\r':
        case '\t':
        case '\n': {
            int i = 0;
            while (data[++i] != 0) {
                if (!is_space(data[i])) break;
            }
            matches[0].rm_eo = i;
            matches[0].rm_so = 0;
            matches[1 + WHITESPACE].rm_eo = i;
            matches[1 + WHITESPACE].rm_so = 0;
            return 0;
        }
        default:
            return 1;
    }

    return 0;
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