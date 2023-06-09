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
    StringT *concat_expected = String_from("Hello, World");

    log_result(__func__, string_t_equals(concat, concat_expected));
}

static void
test_concatenate_inplace() {
    StringT *str = String_from("Hello, ");
    StringT *str2 = String_from("World");
    StringT *concat_expected = String_from("Hello, World");

    String_concatenate_inplace(str, str2);

    log_result(__func__, string_t_equals(str, concat_expected));
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
    StringT *str = String_from("Hello 123");
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
test_count() {
    StringT *str = String_from("Hello, World");
    StringT *str2 = String_from("l");
    StringT *str3 = String_from("lo");

    log_result(__func__, String_count(str, str2) == 3 && String_count(str, str3) == 1);
    ssize_t count_str2 = String_count(str, str2);
    ssize_t count_str3 = String_count(str, str3);
    ssize_t count_str2_expected = 3;
    ssize_t count_str3_expected = 1;

    log_result(__func__, count_str2 == count_str2_expected && count_str3 == count_str3_expected);
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
    int contains_str2 = String_contains_in_range(str, str2, StringIndex(0, 4, 1)).stop;
    int contains_str3 = !String_contains_in_range(str, str3, StringIndex(0, 4, 1)).stop;

    log_result(__func__, contains_str2 && contains_str3);
}

static void
test_reverse() {
    StringT *str = String_from("Hello, World!");
    StringT *reversed = String_reverse(str);
    StringT *reversed_expected = String_from("!dlroW ,olleH");

    log_result(__func__, string_t_equals(reversed, reversed_expected));
}

static void
test_join() {
    StringIteratorT *iter = StringIterator_new();
    StringIterator_append(iter, String_from("Foo"));
    StringIterator_append(iter, String_from("Bar"));
    StringIterator_append(iter, String_from("Spam"));
    StringIterator_append(iter, String_from("Egg"));

    StringT *joined = String_join(iter, String_from("-"));
    StringT *joined_expected = String_from("Foo-Bar-Spam-Egg");

    log_result(__func__, string_t_equals(joined, joined_expected));
}

static void
test_slice() {
    StringT *str = String_from("foo bar");
    StringIndexT idx = StringIndex(4);
    StringT *slice = String_slice(str, idx);
    StringT *slice_expected = String_from("foo ");

    log_result(__func__, string_t_equals(slice, slice_expected));
}

static void
test_repeat() {
    StringT *str = String_from("foo ");
    StringT *rep_pos = String_repeat(str, 5);
    StringT *rep_neg = String_repeat(str, -1);
    StringT *rep_pos_expected = String_from("foo foo foo foo foo ");
    StringT *rep_neg_expected = String_new(0);

    log_result(__func__, string_t_equals(rep_pos, rep_pos_expected) &&
                             string_t_equals(rep_neg, rep_neg_expected));
}

static void
test_to_upper() {
    StringT *str = String_from("Foo Bar");
    StringT *upper = String_to_upper(str);
    StringT *upper_expected = String_from("FOO BAR");

    log_result(__func__, string_t_equals(upper, upper_expected));
}

static void
test_to_lower() {
    StringT *str = String_from("Foo Bar");
    StringT *lower = String_to_lower(str);
    StringT *lower_expected = String_from("foo bar");

    log_result(__func__, string_t_equals(lower, lower_expected));
}

static void
test_to_title() {
    StringT *str = String_from("foo bar");
    StringT *title = String_to_title(str);
    StringT *title_expected = String_from("Foo Bar");

    log_result(__func__, string_t_equals(title, title_expected));
}

static void
test_to_capital() {
    StringT *str = String_from("foo bar");
    StringT *capital = String_to_capital(str);
    StringT *capital_expected = String_from("Foo bar");

    log_result(__func__, string_t_equals(capital, capital_expected));
}

static void
test_swap_case() {
    StringT *str = String_from("Foo Bar");
    StringT *swap_case = String_swap_case(str);
    StringT *swap_case_expected = String_from("fOO bAR");

    log_result(__func__, string_t_equals(swap_case, swap_case_expected));
}

static void
test_trim_whitespace() {
    StringT *str = String_from(" Foo Bar  ");
    StringT *trim = String_trim_whitespace(str);
    StringT *trim_expected = String_from("Foo Bar");

    log_result(__func__, string_t_equals(trim, trim_expected));
}

static void
test_centre() {
    StringT *str = String_from("Foo Bar");
    StringT *centre = String_centre(str, 10);
    StringT *centre_expected = String_from(" Foo Bar  ");

    log_result(__func__, string_t_equals(centre, centre_expected));
}

static void
test_left_justify() {
    StringT *str = String_from("Foo Bar");
    StringT *left_justify = String_left_justify(str, 10);
    StringT *left_justify_expected = String_from("Foo Bar   ");

    log_result(__func__, string_t_equals(left_justify, left_justify_expected));
}

static void
test_right_justify() {
    StringT *str = String_from("Foo Bar");
    StringT *right_justify = String_right_justify(str, 10);
    StringT *right_justify_expected = String_from("   Foo Bar");

    log_result(__func__, string_t_equals(right_justify, right_justify_expected));
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
    test_count();
    test_contains();
    test_contains_in_range();
    test_reverse();
    test_join();
    test_slice();
    test_repeat();
    test_to_upper();
    test_to_lower();
    test_to_title();
    test_to_capital();
    test_swap_case();
    test_trim_whitespace();
    test_centre();
    test_left_justify();
    test_right_justify();
}
