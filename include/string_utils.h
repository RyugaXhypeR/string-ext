#ifndef UNIT_TEST_UTILS_H
#define UNIT_TEST_UTILS_H

#include "string_ext.h"

#include <stdarg.h>
#include <stdio.h>

#define STRING_FREE_MULTIPLE(...)                                                        \
    string_free_multiple(__NUM_ARGS(StringT *, __VA_ARGS__), __VA_ARGS__)
#define STRING_ITERATOR__FREE_MULTIPLE(...)                                              \
    string_iterator_free_multiple(__NUM_ARGS(StringIteratorT *, __VA_ARGS__), __VA_ARGS__)


void string_free_multiple(int num_args, ...);
void string_iterator_free_multiple(int num_args, ...);
int string_t_equals(StringT *str1, StringT *str2);
int string_equals(const char *str1, const char *str2);
void log_result(const char *method_name, int result);
int string_index_equal(const StringIndexT idx1, const StringIndexT idx2);
void string_free_multiple(int num_args, ...);
void string_iterator_free_multiple(int num_args, ...);

#endif  // UNIT_TEST_UTILS_H
