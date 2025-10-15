#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "splicestr.h"

#define TC(count, tp, ...) test[count] = tp; int a##count[] = {__VA_ARGS__}; testA[count] = a##count;

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
    ERROR
};

const char* int_to_token(int t) {
    switch (t) {
        case OPEN_COMP: return "OPEN_COMP"; break;
        case CLOSE_COMP: return "CLOSE_COMP"; break;
        case OPEN_BR: return "OPEN_BR"; break;
        case CLOSE_BR: return "CLOSE_BR"; break;
        case OPEN_PAREN: return "OPEN_PAREN"; break;
        case CLOSE_PAREN: return "CLOSE_PAREN"; break;
        case COLON: return "COLON"; break;
        case COMMA: return "COMMA"; break;
        case EQUALS: return "EQUALS"; break;
        case HASH: return "HASH"; break;
        case IDENTIFIER: return "IDENTIFIER"; break;
        case NUMBER: return "NUMBER"; break;
        case LITERAL: return "LITERAL"; break;
        case WHITESPACE: return "WHITESPACE"; break;
    }
    return "UNDEFINED";
}

int main() {
    struct ruleset rs;
    const int test_count = 16;

    const char* test[test_count];
    int* testA[test_count];


    TC(0, "hello world", IDENTIFIER, WHITESPACE, IDENTIFIER, -1);
    TC(1, " \r\t", WHITESPACE, -1);
    TC(2, "<hello>", OPEN_COMP, IDENTIFIER, CLOSE_COMP, -1);
    TC(3, "<{hello}>", OPEN_COMP, LITERAL, CLOSE_COMP, -1);
    TC(4, "<[h:1]{Heading}>", OPEN_COMP, OPEN_BR, IDENTIFIER, COLON, NUMBER, CLOSE_BR, LITERAL, CLOSE_COMP, -1);
    TC(5, "<{literal}> [:=] identifier(identifier,26,#a12)", OPEN_COMP, LITERAL, CLOSE_COMP, WHITESPACE, OPEN_BR, COLON, EQUALS, CLOSE_BR, WHITESPACE, IDENTIFIER, OPEN_PAREN, IDENTIFIER, COMMA, NUMBER, COMMA, HASH, IDENTIFIER, CLOSE_PAREN, -1);
    TC(6, "{emoji✅ literals😂}", LITERAL, -1);
    TC(7, "{not a literal either", ERROR, 0);
    TC(8, "{árvíztűrő tükörfúrógép} {&x7B;} {&x7D;}", LITERAL, WHITESPACE, LITERAL, WHITESPACE, LITERAL, -1);
    TC(9, "\t{⎧⎨⎩}\r\rS",WHITESPACE, LITERAL, WHITESPACE, IDENTIFIER, -1);
    TC(10, "{{}", ERROR, 0);
    TC(11, "{}}", LITERAL, ERROR, 2);
    TC(12, "{a}{b}", LITERAL, LITERAL, -1);

    TC(13, "not✅ a literal}", IDENTIFIER, ERROR, 3);
    TC(14, "árvíztűrő tükörfúrógép", ERROR, 0);
    TC(15, "not✅identifier", IDENTIFIER, ERROR, 3);

    rs.rules[0] = "<";
    rs.rules[1] = ">";
    rs.rules[2] = "\\[";
    rs.rules[3] = "\\]";
    rs.rules[4] = "\\(";
    rs.rules[5] = "\\)";
    rs.rules[6] = ":";
    rs.rules[7] = ",";
    rs.rules[8] = "=";
    rs.rules[9] = "#";
    rs.rules[10] = "[a-zA-Z_][a-zA-Z0-9_$]*";
    rs.rules[11] = "[0-9]+";
    rs.rules[12] = "\\{[^{}]*\\}";
    rs.rules[13] = "[ \t\n\r]+";
    rs.length = 14;

    struct splicestr input;
    struct splicestr match;
    int success = 0;
    int failure = 0;

    compile_ruleset(&rs);
    for (int i = 0; i < test_count; i++) {
        struct lexer lex;
        lex.rules = &rs;
        lex.input = &input;
        create_lexer(&lex, test[i]);
        int token_index;
        int index = 0;
        bool allMatch = true;
        while ((token_index = get_next_token(&lex, &match)) >= 0) {
            if (testA[i][index] == token_index) {
#ifdef DEBUG                            
                printf("\033[32m.\033[0m");
#endif
#ifdef DEBUG_TOKENS
                printf("\033[44m%.*s\033[0m ",match.length, match.data + match.start);
#endif
            } else {
#ifdef DEBUG
                printf("\033[31m.\033[0m(%d %d)", token_index, testA[i][index]);
#endif
#ifdef DEBUG_TOKENS
                printf("\033[43m%.*s\033[0m ",match.length, match.data + match.start);
#endif
                allMatch = false;
            }
            index++;
        }
        if (testA[i][index] != -1) allMatch = false;
        if (token_index == -3) {
            if (allMatch) {
                success++;
                printf("Successfully lexed \033[42m%s\033[0m\n", test[i]);
            } else {
                failure++;
                printf("Successfully lexed \033[43m%s\033[0m with mismatched rules\n", test[i]);
            }
        } else {
#ifdef DEBUG_TOKENS
            printf("\033[41m%.*s\033[0m ",match.length, match.data + match.start);
#endif            
            if (testA[i][index+1] == lex.position) {
                success++;
                printf("Expected syntax error \033[42m%s\033[m: error near column %d (%d)\n", test[i], lex.position, token_index);
            } else {
                failure++;
                printf("Unexpected syntax error \033[43m%s\033[m: error near column %d instead of %d (%d)\n", test[i], lex.position, testA[i][index+1], token_index);
            }
            
        }
        destroy_lexer(&lex);
    }
    regfree(&rs.regex);

    printf("Test finished with \033[32m%d\033[0m successful cases and \033[31m%d\033[0m failing\n", success, failure);
    if (failure != 0) {
        return 1;
    }
}
