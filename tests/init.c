/// Tests the initlialization of the `StringT` object.

#include "../include/str.h"
#include "../include/utils.h"

void
test_string_new() {
    ssize_t size = 10;
    StringT *string = String_new(size);

    log_result(__func__, string->allocated == size);
    STRING_FREE_MULTIPLE(string);
}

void
test_string_from() {
    const char *str = "Hello, World!";
    StringT *string = String_from(str);

    log_result(__func__, string_equals(str, string->string));
    STRING_FREE_MULTIPLE(string);
}

int
main() {
    test_string_new();
    test_string_from();
}
