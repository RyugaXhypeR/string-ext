#include "../include/string.h"
#include "../include/utils.h"

#include <stdio.h>

static void
test_copy() {
    StringT *str = String_from("Hello, World");
    StringT *copy = String_copy(str);

    log_result(__func__, string_t_equals(str, copy));
}

static void
test_concatenate() {
    StringT *str = String_from("Hello, ");
    StringT *str2 = String_from("World");
    StringT *concat = String_concatenate(str, str2);

    log_result(__func__, string_t_equals(concat, String_from("Hello, World")));
}

static void
test_concatenate_inplace() {
    StringT *str = String_from("Hello, ");
    StringT *str2 = String_from("World");
    String_concatenate_inplace(str, str2);

    log_result(__func__, string_t_equals(str, String_from("Hello, World")));
}

static void
test_equals() {
    StringT *str = String_from("Hello, World");
    StringT *str2 = String_from("Hello, World");

    log_result(__func__, String_equals(str, str2));
}

static void
test_ends_with() {
    StringT *str = String_from("Hello, World");
    StringT *str2 = String_from("World");
    StringT *str3 = String_from("Hello");

    log_result(__func__, String_ends_with(str, str2) && !String_ends_with(str, str3));
}

static void
test_starts_with() {
    StringT *str = String_from("Hello, World");
    StringT *str2 = String_from("Hello");
    StringT *str3 = String_from("World");

    log_result(__func__, String_starts_with(str, str2) && !String_starts_with(str, str3));
}

static void
test_is_alphanumeric() {
    StringT *str = String_from("Hello");
    StringT *str2 = String_from("Hello123");
    log_result(__func__, !String_is_alphanumeric(str) && String_is_alphanumeric(str2));
}

static void
test_is_uppercase() {
    StringT *str = String_from("HELLO, WORLD");
    StringT *str2 = String_from("Hello, World");

    log_result(__func__, String_is_uppercase(str) && !String_is_uppercase(str2));
}

static void
test_is_lowercase() {
    StringT *str = String_from("hello, world");
    StringT *str2 = String_from("Hello, World");

    log_result(__func__, String_is_lowercase(str) && !String_is_lowercase(str2));
}

static void
test_is_numeric() {
    StringT *str = String_from("123");
    StringT *str2 = String_from("Hello, World");

    log_result(__func__, String_is_numeric(str) && !String_is_numeric(str2));
}

static void
test_is_decimal() {
    StringT *str = String_from("123.123");
    StringT *str2 = String_from("Hello, World");

    log_result(__func__, String_is_decimal(str) && !String_is_decimal(str2));
}

static void
test_is_whitespace() {
    StringT *str = String_from(" ");
    StringT *str2 = String_from("Hello, World");

    log_result(__func__, String_is_whitespace(str) && !String_is_whitespace(str2));
}

static void
test_contains() {
    StringT *str = String_from("Hello, World");
    StringT *str2 = String_from("l");
    StringT *str3 = String_from("lo");

    log_result(__func__,
               String_contains(str, str2).stop && String_contains(str, str3).stop);
}

static void
test_contains_in_range() {
    StringT *str = String_from("Hello, World");
    StringT *str2 = String_from("l");
    StringT *str3 = String_from("lo");

    log_result(__func__,
               String_contains_in_range(str, str2, StringIndex(0, 4, 1)).stop &&
                   !String_contains_in_range(str, str3, StringIndex(0, 4, 1)).stop);
}

int
main() {
    test_copy();
    test_concatenate();
    test_concatenate_inplace();
    test_equals();
    test_ends_with();
    test_starts_with();
    test_is_alphanumeric();
    test_is_uppercase();
    test_is_lowercase();
    test_is_numeric();
    test_is_decimal();
    test_is_whitespace();
    test_contains();
    test_contains_in_range();
}
