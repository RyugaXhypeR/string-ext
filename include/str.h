#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stdlib.h> /* ssize_t */

typedef struct {
    char *string;

    ssize_t length;
    ssize_t allocated;
} StringT;

typedef struct {
    int start;
    int stop;
    int step;
} StringIndexT;

typedef struct {
    const StringT **strings;

    ssize_t index;
    ssize_t length;
    ssize_t allocated;
} StringIteratorT;

StringT *String_new(ssize_t size);
StringT *String_from(const char *_string);

char String_index(const StringT *self, ssize_t index);
void String_concatenate_inplace(StringT *self, const StringT *other);
bool String_equals(const StringT *self, const StringT *other);
bool String_ends_with(const StringT *self, const StringT *suffix);
bool String_starts_with(const StringT *self, const StringT *prefix);
bool String_is_alphanumeric(const StringT *self);
bool String_is_uppercase(const StringT *self);
bool String_is_lowercase(const StringT *self);
bool String_is_int(const StringT *self);
bool String_is_real(const StringT *self);
bool String_is_whitespace(const StringT *self);
ssize_t String_count(const StringT *self, const StringT *sub_string);
StringIndexT String_contains(const StringT *self, const StringT *sub_string);
StringIndexT String_contains_in_range(const StringT *self, const StringT *other,
                                      StringIndexT index);
StringIndexT String_contains_char(const StringT *self, const char character);
StringIndexT String_contains_char_in_range(const StringT *self, const char character,
                                           StringIndexT index);
StringIndexT String_find_from_char_class(const StringT *self, const StringT *characters);
StringIndexT String_find_from_char_class_in_range(const StringT *self,
                                                  const StringT *characters,
                                                  StringIndexT index);
StringIteratorT *String_split(const StringT *self, const StringT *delimiter);
StringIteratorT *String_split_lines(const StringT *self);
StringIteratorT *String_split_whitespace(const StringT *self);
StringIteratorT *String_split_limit(const StringT *self, const StringT *delimiter,
                                    ssize_t limit);
StringIteratorT *String_split_lines_limit(const StringT *self, ssize_t limit);
StringIteratorT *String_split_whitespace_limit(const StringT *self, ssize_t limit);
StringIteratorT *String_split_in_range(const StringT *self, const StringT *delimiter,
                                       StringIndexT index);
StringIteratorT *String_right_split(const StringT *self, const StringT *delimiter);
StringIteratorT *String_right_split_limit(const StringT *self, const StringT *delimiter,
                                          ssize_t limit);
StringT *String_copy(const StringT *self);
StringT *String_join(StringIteratorT *self, const StringT *delimiter);
StringT *String_slice(const StringT *self, StringIndexT index);
StringT *String_concatenate(const StringT *self, const StringT *other);
StringT *String_repeat(const StringT *self, ssize_t times);
StringT *String_replace(const StringT *self, const StringT *sub_string,
                        const StringT *replacement);
StringT *String_reverse(const StringT *self);
StringT *String_to_upper(const StringT *self);
StringT *String_to_lower(const StringT *self);
StringT *String_to_title(const StringT *self);
StringT *String_to_capital(const StringT *self);
StringT *String_swap_case(const StringT *self);
StringT *String_trim_whitespace(const StringT *self);
StringT *String_trim_left(const StringT *self);
StringT *String_trim_right(const StringT *self);
StringT *String_format(const StringT *self, ...);
StringT *String_to_int(const StringT *self);
StringT *String_to_float(const StringT *self);
StringT *String_centre(const StringT *self, ssize_t width);
StringT *String_left_justify(const StringT *self, ssize_t width);
StringT *String_right_justify(const StringT *self, ssize_t width);
StringT *String_fill(const StringT *self, ssize_t width);
StringIteratorT *String_chunks(const StringT *self, ssize_t chunk_size);

void String_free(StringT *self);

/* StringIteratorT */
StringIteratorT *StringIterator_new();
const StringT *StringIterator_next(StringIteratorT *self);
const StringT *StringIterator_get(StringIteratorT *self);
void StringIterator_append(StringIteratorT *self, const StringT *string);
void StringIterator_free(StringIteratorT *self);

/* StringIndexT */
// Helper macro to get number of arguments passed to a macro.
#define __NUM_ARGS(type, ...) sizeof((type[]){__VA_ARGS__}) / sizeof(type)

/// Helper macro to initialize `StringIndexT` object.
/// When only one argument is passed, it is used as `stop` and `start` is set to 0.
/// When two arguments are passed, they are used as `start` and `stop` respectively.
/// When three arguments are passed, they are used as `start`, `stop` and `step`
/// respectively.
///
/// # Example
/// ```c
/// StringIndexT index_with_stop = StringIndex(10);
/// StringIndexT index_with_start_stop = StringIndex(0, 10);
/// StringIndexT index_with_start_stop_step = StringIndex(0, 10, 2);
/// ```
#define StringIndex(...)                                                                 \
    StringIndex__init__(__NUM_ARGS(ssize_t, __VA_ARGS__), __VA_ARGS__)
StringIndexT StringIndex__init__(size_t nargs, ...);
StringIndexT StringIndex_new(ssize_t start, ssize_t stop, ssize_t step);

#endif /* STRING_H */
