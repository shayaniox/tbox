#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

struct string {
    char *data;
    size_t len;
    size_t cap;
};

string *str_init();
int str_append(string *str, char *chars, size_t l);
int str_free(string *str);

#endif
