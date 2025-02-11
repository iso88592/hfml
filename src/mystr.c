#include "mystr.h"

#include <assert.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

const int MYSTR_INITIAL_BLOCK_COUNT = 1;
const int MYSTR_BLOCK_COUNT = 64;

void mystr_create(struct mystr* self) {
    self->parts = (char**)malloc(sizeof(char*) * MYSTR_INITIAL_BLOCK_COUNT);
    self->partCount = 0;
    self->partCapacity = MYSTR_INITIAL_BLOCK_COUNT;
}

struct mystr* mystr_construct() {
    struct mystr* result = (struct mystr*)malloc(sizeof(struct mystr));
    mystr_create(result);
    return result;
}

struct mystr* mystr_construct_s(const char* str) {
    struct mystr* result = mystr_construct();
    mystr_append(result, str);
    return result;
}


void mystr_destroy(struct mystr* self) {
    for (int i = 0; i < self->partCount; i++) {
        free(self->parts[i]);
    }
    free(self->parts);
}

void mystr_append(struct mystr* self, const char* str) {
    if (self->partCount >= self->partCapacity) {
        self->partCapacity += MYSTR_BLOCK_COUNT;
        self->parts = (char**)realloc(self->parts, sizeof(char*) * self->partCapacity);
    }
    int nl = strlen(str) + 1;
    char* newString = (char*)malloc(nl);
    strncpy(newString, str, nl);
    self->parts[self->partCount] = newString;
    self->partCount++;
}

struct mystr* mystr_consume(struct mystr* left, struct mystr* right) {
    int totalSize = left->partCount + right->partCount;
    if (totalSize >= left->partCapacity) {
        while (totalSize >= left->partCapacity) {
            left->partCapacity += MYSTR_BLOCK_COUNT;
        }
        left->parts = (char**)realloc(left->parts, sizeof(char*) * left->partCapacity);
    }
    for (int i = 0; i < right->partCount; i++) {
        mystr_append(left, right->parts[i]);
    }
    mystr_destroy(right);
    return left;
}

char* mystr_to_c(struct mystr* self) {
    int size = 1;
    for (int i = 0; i < self->partCount; i++) {
        size += strlen(self->parts[i]);
    }
    char* newString = (char*)malloc(size);
    bzero(newString, size);
    int current = 0;
    for (int i = 0; i < self->partCount; i++) {
        int n = strlen(self->parts[i]);
        strncpy(newString + current, self->parts[i], size - current);
        current += n;
    }
    assert(strlen(newString) == size - 1);
    return newString;
}
