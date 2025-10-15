#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "splicestr.h"

void compile_ruleset(struct ruleset* ruleset) {
    int cs = SPLICESTR_REGEX_SIZE;
#ifdef SPLICESTR_REGEX_HEAP    
    char* result = (char*)SPLICESTR_MALLOC(cs);
#else    
    char result[cs];
#endif    
    size_t cp = 0;
    for (int i = 0; i < ruleset->length; i++) {
        if (cp == 0) {
            cp += snprintf(result, cs, "(^%s)", ruleset->rules[i]);
        } else {
            cp += snprintf(result + cp, cs - cp, "|^(%s)", ruleset->rules[i]);
        }
        assert(cp < SPLICESTR_REGEX_SIZE);
    }
#ifdef DEBUG
    printf("%s\n", result);
#endif
    regcomp(&ruleset->regex, result, REG_EXTENDED | REG_NEWLINE);

#ifdef SPLICESTR_REGEX_HEAP
    SPLICESTR_FREE(result);
#endif
}

void splicestr_create(const char* source, struct splicestr* str) {
    str->length = strlen(source);
#ifdef SPLICESTR_CONST
    str->data = source;
#else    
    str->data = (char*)SPLICESTR_MALLOC(str->length + 1);
    bzero(str->data, str->length + 1);
    memcpy(str->data, source, str->length + 1);
#endif    
    str->start = 0;
}

void splicestr_destroy(struct splicestr* str) {
    assert(str->start == 0);
#ifndef SPLICESTR_CONST
    bzero(str->data, str->length);
    SPLICESTR_FREE(str->data);
#endif    
}

void splicestr_substr(const struct splicestr* source, SPLICESTR_INT from, SPLICESTR_INT length, struct splicestr* result) {
    result->data = source->data;
    result->start = from;
    result->length = length;
}

void create_lexer(lexer* lex, const char* input) {
    splicestr_create(input, lex->input);
    lex->position = 0;
}

void destroy_lexer(lexer* lex) {
    splicestr_destroy(lex->input);
}

int get_next_token(lexer* lex, struct splicestr* splice) {
    if (lex->position == lex->input->length) return -3;
    regmatch_t matches[lex->rules->length + 1];
    bzero(&matches, sizeof(regmatch_t)*(lex->rules->length + 1));
    int r = regexec(&lex->rules->regex, lex->input->data + lex->position, lex->rules->length+1, matches, 0);
    if (r == 0) {
        for (int i = 1; i < lex->rules->length+1; i++) {
            if (matches[0].rm_eo == matches[i].rm_eo) {
                assert(matches[i].rm_so == 0);
                splicestr_substr(lex->input, lex->position, matches[i].rm_eo, splice);
                lex->position += matches[i].rm_eo;
                return i-1;
            }
        }
        splicestr_substr(lex->input, lex->position, lex->input->length - lex->position, splice);
        return -1;
    }
    splicestr_substr(lex->input, lex->position, lex->input->length - lex->position, splice);
    return -2;
}

int splicestr_atoi(const struct splicestr* str) {
    if (str->length == 0) return 0;
    int result = 0;
    for (int i = str->start; i < str->start + str->length; i++) {
        if (str->data[i] >= '0' && str->data[i] <= '9') {
            result *= 10;
            result += str->data[i]-'0';
        }
    }
    if (str->data[str->start] == '-') {
        result *= -1;
    }
    return result;
}
