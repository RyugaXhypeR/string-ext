#include "str_utils.h"

#include "str.h"

int
string_t_equals(StringT *str1, StringT *str2) {
    if (str1->length != str2->length) return 0;
    for (int i = 0; i < str1->length; i++)
        if (str1->string[i] != str2->string[i]) return 0;

    return 1;
}

int
string_equals(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1++ != *str2++) return 0;
    }
    return *str1 == *str2;
}

void
log_result(const char *method_name, int result) {
    if (result)
        fprintf(stdout, "[OK]: %s\n", method_name);
    else
        fprintf(stderr, "[ERR]: %s\n", method_name);
}

int
string_index_equal(const StringIndexT idx1, const StringIndexT idx2) {
    return idx1.start == idx2.start && idx1.stop == idx2.stop && idx1.step == idx2.step;
}

void
string_free_multiple(int num_args, ...) {
    va_list args;
    va_start(args, num_args);

    while (num_args--) {
        StringT *str = va_arg(args, StringT *);
        String_free(str);
    }

    va_end(args);
}

void
string_iterator_free_multiple(int num_args, ...) {
    va_list args;
    va_start(args, num_args);

    while (num_args--) {
        StringIteratorT *iter = va_arg(args, StringIteratorT *);
        StringIterator_free(iter);
    }

    va_end(args);
}
