#include "include/string.h"

#include <stdio.h>

int
main()
{
    StringT *string = String_from("Hello, World!");
    StringT *lines = String_from("Hello  world\n\nyoyo\nWorld!");
    StringT *new = String_new(0);

    StringIteratorT *iterator = String_split(string, String_from(", "));
    StringIteratorT *liness = String_split_whitespace(lines);

    printf("%d\n", String_starts_with(string, String_from("Hello")));
    printf("%d\n", String_ends_with(string, String_from("!")));
    printf("%s\n", String_reverse(string)->string);
    printf("%s\n", String_swap_case(string)->string);

    String_free(string);
    String_free(lines);
    String_free(new);
}
