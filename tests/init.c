/// Tests the initlialization of the `StringT` object.

#include "../include/string.h"

#include <assert.h>
#include <string.h>

void
compare_string(StringT *string, const char *str) {
    for (ssize_t i = 0; i < string->length; i++) assert(string->string[i] == str[i]);
}

void
test_string_new(ssize_t size) {
    StringT *string = String_new(size);
    assert(string->allocated == size);
    String_free(string);
}

void
test_string_from(const char *str) {
    StringT *string = String_from(str);
    assert(string->length == strlen(str));
    compare_string(string, str);
    String_free(string);
}

int
main() {
    test_string_new(10);
    test_string_from("Hello World!");
    return 0;
}
