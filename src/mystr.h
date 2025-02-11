#ifndef __MYSTR__H__
#define __MYSTR__H__

struct mystr {
    char** parts;
    int partCount;
    int partCapacity;
};

extern struct mystr* mystr_construct();
extern struct mystr* mystr_construct_s(const char*);
extern void mystr_create(struct mystr*);
extern void mystr_destroy(struct mystr*);
extern void mystr_append(struct mystr*, const char*);
extern struct mystr* mystr_consume(struct mystr*, struct mystr*);
extern char* mystr_to_c(struct mystr*);

#endif