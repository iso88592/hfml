#include "splicestr_config.h"
#ifndef SPLICESTR_H
#define SPLICESTR_H 

#ifndef SPLICESTR_MALLOC
#define SPLICESTR_MALLOC malloc
#endif

#ifndef SPLICESTR_FREE
#define SPLICESTR_FREE free
#endif

#ifndef SPLICESTR_CALLOC
#define SPLICESTR_CALLOC calloc
#endif

#ifndef SPLICESTR_ZERO
#define SPLICESTR_ZERO bzero
#endif

#ifndef SPLICESTR_INT
#define SPLICESTR_INT unsigned int
#endif

#ifndef SPLICESTR_MAX_RULE_COUNT
#define SPLICESTR_MAX_RULE_COUNT 64
#endif

#ifndef SPLICESTR_REGEX_SIZE
#define SPLICESTR_REGEX_SIZE 4096
#endif

struct splicestr {
#ifdef SPLICESTR_CONST
    const char* data;
#else
    char* data;
#endif
    SPLICESTR_INT start;
    SPLICESTR_INT length;
};

struct ruleset {
    const char* rules[SPLICESTR_MAX_RULE_COUNT];
    int length;
    regex_t regex;
};

struct lexer {
    struct ruleset* rules;
    struct splicestr* input;
    size_t position;
};

void compile_ruleset(struct ruleset* ruleset);
void splicestr_create(const char* source, struct splicestr* str);
void splicestr_destroy(struct splicestr* str);
void splicestr_substr(const struct splicestr* source, SPLICESTR_INT from, SPLICESTR_INT length, struct splicestr* result);
void create_lexer(struct lexer* lex, const char* input);
void destroy_lexer(struct lexer* lex); 
int get_next_token(struct lexer* lex, struct splicestr* splice); 
int splicestr_atoi(const struct splicestr* str);

#endif
