/// Tests the initlialization of the `StringT` object.

#include "../include/string.h"
#include "utils.h"

#include <assert.h>
#include <string.h>

void
test_string_new() {
    ssize_t size = 10;
    StringT *string = String_new(size);

    log_result(__func__, string->length == size);
    String_free(string);
}

void
test_string_from() {
    const char *str = "Hello, World!";
    StringT *string = String_from(str);

    log_result(__func__, string_equals(str, string->string));
    String_free(string);
}

int
main() {
    test_string_new();
    test_string_from();
}
