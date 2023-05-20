#ifndef UNIT_TEST_UTILS_H
#define UNIT_TEST_UTILS_H

#include "../include/string.h"

#include <stdio.h>

inline int
string_t_equals(StringT *str1, StringT *str2) {
    if (str1->length != str2->length) return 0;
    for (int i = 0; i < str1->length; i++)
        if (str1->string[i] != str2->string[i]) return 0;

    return 1;
}

inline int
string_equals(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1++ != *str2++) return 0;
    }
    return *str1 == *str2;
}

inline void
log_result(const char *method_name, int result) {
    if (result)
        fprintf(stdout, "[OK]: %s\n", method_name);
    else
        fprintf(stderr, "[ERR]: %s\n", method_name);
}

#endif  // UNIT_TEST_UTILS_H
