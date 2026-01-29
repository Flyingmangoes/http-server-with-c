#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

BString *filename = NULL;

BString *bString_Init(size_t capacity, const char *const s) {
    struct BString *bstr = malloc(sizeof(struct BString));
    if (bstr == NULL) return NULL;
    if(capacity == 0) capacity = BSTRING_INIT_CAPACITY;

    const size_t stringlen = (s == NULL) ? 0 : strlen(s);
    if(stringlen >= capacity) {
        capacity = stringlen + 1;
    }

    bstr->data = malloc(sizeof(char) * capacity);
    if(bstr->data == NULL) {
        free(bstr);
        return NULL;
    }

    if(s != NULL) {
        memcpy(bstr->data, s, stringlen);
    }

    bstr->capacity = capacity;
    bstr->length = stringlen;
    bstr->data[stringlen] = '\0';

    return bstr;
};

bool bString_Appends(BString *self, const char *const s) {
    assert(s != NULL);
    assert(self != NULL);

    const size_t stringlen = strlen(s);
    const size_t new_len = self->length + stringlen;
    if(new_len >= self->capacity) {
        size_t new_cap = self->capacity * 2;
        if(new_len >= new_cap) {
            new_cap = new_len + 1;
        }

        char *new_data = realloc(self->data, new_cap);
        if (new_data == NULL) return false;
        
        self->data = new_data;
        self->capacity = new_cap;
    }
    
    memcpy(&self->data[self->length], s, stringlen);
    self->length = new_len;
    self->data[new_len] = '\0';

    return true;
};

void bString_Free(BString *self) {
    assert(self != NULL);

    free(self->data);
    free(self);
};
