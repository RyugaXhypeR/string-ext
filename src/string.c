#include "../include/string.h"

#include <stdio.h>
#include <stdlib.h>

#define CHAR_IS_WHITESPACE(ch)                                                           \
    ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')
#define CHAR_IS_DIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define CHAR_IS_ALPHABET(ch)                                                             \
    (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))
#define CHAR_IS_LOWERCASE(ch) ((ch) >= 'a' && (ch) <= 'z')
#define CHAR_IS_UPPERCASE(ch) ((ch) >= 'A' && (ch) <= 'Z')
#define CHAR_IS_ALPHA_NUMERIC(ch) (CHAR_IS_ALPHABET(ch) || CHAR_IS_DIGIT(ch))

#define CHAR_TO_LOWERCASE(ch)                                                            \
    if (CHAR_IS_ALPHABET(ch)) ch |= 0x20
#define CHAR_TO_UPPERCASE(ch)                                                            \
    if (CHAR_IS_ALPHABET(ch)) ch &= ~0x20
#define CHAR_SWAP_CASE(ch)                                                               \
    if (CHAR_IS_ALPHABET(ch)) ch ^= 0x20

/// Convert negative index to C-valid index.
/// If index is out of range, raise an exception.
static ssize_t
negative_index_to_positive(ssize_t index, size_t length) {
    if (index < 0) index += length;

    if (index < 0 || index >= (ssize_t)length) {
        fprintf(stderr, "Index out of range");
        exit(EXIT_FAILURE);
    }

    return index;
}

static size_t
c_string_length(const char *string) {
    size_t length = 0;

    while (*string++) length++;

    return length;
}

/* ------------------------------ StringIteratorT ------------------------------ */

/// Iterator for `StringT` object.
/// Basically a vector storing the stirngs.
StringIteratorT *
StringIterator_new() {
    StringIteratorT *self = malloc(sizeof *self);
    StringT **string_array = malloc(sizeof **string_array);
    *self = (StringIteratorT){
        .strings = string_array, .index = 0, .length = 0, .allocated = 1};
    return self;
}

StringT *
StringIterator_next(StringIteratorT *self) {
    if (self->index >= self->length) return NULL;
    return self->strings[self->index++];
}

StringT *
StringIterator_get(StringIteratorT *self) {
    return *self->strings;
}

void
StringIterator_free(StringIteratorT *self) {
    free(self->strings);
    free(self);
}

void
StringIterator_append(StringIteratorT *self, StringT *string) {
    if (self->length >= self->allocated) {
        self->allocated <<= 1;
        self->strings = realloc(self->strings, self->allocated * sizeof *self->strings);
    }
    self->strings[self->length++] = string;
}

/* ------------------------------ StringIndexT ------------------------------ */
StringIndexT
StringIndex_new(ssize_t start, ssize_t stop, ssize_t step) {
    if (step == 0) {
        fprintf(stderr, "Step cannot be 0");
        exit(EXIT_FAILURE);
    }
    return (StringIndexT){.start = start, .stop = stop, .step = step};
}

static void
StringIndex_convert_to_postive(StringIndexT self, size_t length) {
    self.start = negative_index_to_positive(self.start, length);
    self.stop = negative_index_to_positive(self.stop, length);
}

/* ------------------------------ StringT ------------------------------ */

/// Re-allocate extra space for the string.
static void
String_re_allocate(StringT *self, ssize_t new_size) {
    ssize_t new_allocated;

    if (new_size <= self->allocated) return;

    new_allocated = (new_size + (new_size >> 3) + 6) & ~3;

    self->string = realloc(self->string, new_allocated * sizeof *self->string);
    self->allocated = new_allocated;
}

static void
String_push(StringT *self, char ch) {
    String_re_allocate(self, self->length + 1);

    self->string[self->length] = ch;
    self->length++;
}

static void
String_shift_left(const StringT *self, ssize_t index, ssize_t length) {
    for (ssize_t i = index; i < length; i++) self->string[i] = self->string[i + 1];
}

static void
String_shift_right(const StringT *self, ssize_t index, ssize_t length) {
    for (ssize_t i = length; i > index; i--) self->string[i] = self->string[i - 1];
}

/// Create an empty `StringT` object with the given size
///
/// # Example
/// ```c
/// StringT *string = String_new(10);
/// ```
StringT *
String_new(ssize_t size) {
    StringT *self = malloc(sizeof *self);
    *self = (StringT){.string = malloc(size), .length = 0, .allocated = size};
    return self;
}

static StringT *
String_from_char_array_with_length(const char *string, size_t length) {
    char *new_string = malloc((length + 1) * sizeof *new_string);
    StringT *self = malloc(sizeof *self);

    for (ssize_t i = 0; i < length; ++i) new_string[i] = string[i];
    new_string[length] = '\0';

    *self = (StringT){.string = new_string, .length = length, .allocated = length};

    return self;
}

StringT *
String_copy(const StringT *self) {
    return String_from(self->string);
}

/// Deep free the `StringT` object and the inner C-string.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// printf("%s", string->string); // Hello, World
/// String_free(string);
/// ```
void
String_free(StringT *self) {
    free(self->string);
    free(self);
}

/// Convert a **valid** C-string to `StringT`
/// The string passed in will not be destroyed and will remain constant.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// ```
StringT *
String_from(const char *_string) {
    return String_from_char_array_with_length(_string, c_string_length(_string));
}

/// Return the character at the given index
/// Supports negative indexing.
///
/// # Exceptions
/// - `Index out of range`: If the index is out of range, both positive and negative
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// char c = String_index(string, 0); // 'H'
/// char c = String_index(string, -1); // '!'
/// ```
char
String_index(const StringT *self, ssize_t index) {
    index = negative_index_to_positive(index, self->length);

    return self->string[index];
}

/// Return s `StringT` which is a slice of the original `StringT`
/// from `start` to `stop` with a step of `step`.
///
/// # Exceptions
/// - `Index out of range`: If the index is out of range, both positive and negative
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *slice = String_slice(string, 0, 5, 1); // `Hello`
/// StringT *slice = String_slice(string, 5, -1, -1); // `olleH`
/// StringT *slice = String_slice(string, 0, 5, 2); // `Hlo`
/// ``
StringT *
String_slice(const StringT *self, StringIndexT index) {
    StringT *slice = String_new(0);
    ssize_t start;

    // Calculate the length of this range and iterate that many times
    int slice_length = (index.stop - index.start) / index.step;

    while (slice_length--) {
        String_push(slice, self->string[index.start]);
        index.start += index.step;
    }

    String_push(slice, '\0');

    return slice;
}

/// Concatenate two StringT objects and return a new StringT.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, ");
/// StringT *string2 = String_from("World!");
/// StringT *concatenated_string = String_concatenate(string, string2); // "Hello, World!"
/// ```
StringT *
String_concatenate(const StringT *self, const StringT *other) {
    StringT *concatenated_string = String_copy(self);

    String_re_allocate(concatenated_string, self->length + other->length);

    for (ssize_t i = 0; i < other->length; ++i)
        concatenated_string->string[i + self->length] = other->string[i];

    concatenated_string->length += other->length;

    return concatenated_string;
}

/// Concatenate the first string with the second one.
///
/// # Example
/// ```c
/// StringT *string1 = String_from("Hello, ");
/// StringT *string2 = String_from("World!");
/// String_concatenate_inplace(string1, string2);
/// string1 // "Hello, World!"
/// ```
void
String_concatenate_inplace(StringT *self, const StringT *other) {
    String_re_allocate(self, self->length + other->length);

    for (ssize_t i = 0; i < other->length; i++)
        self->string[self->length++] = other->string[i];
}

/// Repeat the string n amount of times.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, ");
/// StringT *repeated_string = String_repeat(string, 3); // "Hello, Hello, Hello, "
/// ```
StringT *
String_repeat(const StringT *self, ssize_t times) {
    ssize_t new_length = self->length * times;
    StringT *new_string = String_from_char_array_with_length(self->string, new_length);

    for (ssize_t i = 1; i < times; i++) String_concatenate_inplace(new_string, self);

    return new_string;
}

/// Check for strict equality between two strings.
/// Time complexity is O(n) when strings are of equal length else O(1)
///
/// # Example
/// ```c
/// StringT *string1 = String_from("Hello, World!");
/// StringT *string2 = String_from("Hello, World!");
/// String_equals(string1, string2); // true
/// ```
bool
String_equals(const StringT *self, const StringT *other) {
    if (self->length != other->length) return false;

    for (ssize_t i = 0; i < self->length; i++)
        if (self->string[i] != other->string[i]) return false;

    return true;
}

/// Check if the string contains a given string.
/// Time complexity is O(n*m) where n is the length of the string and m is the length of
/// the sub_string, when the `other` is smaller than `self`.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *sub_string = String_from("World");
/// String_contains(string, sub_string); // (StringIndexT){7, 12, 1}
/// ```
StringIndexT
String_contains(const StringT *self, const StringT *other) {
    StringIndexT index;
    StringIndexT not_found = (StringIndexT){0, 0, 1};
    ssize_t offset = other->length - 1;

    if (self->length < other->length) return not_found;

    for (ssize_t i = 0; i < self->length - offset; ++i) {
        // Checking if the first and last characters match
        // If so, then we construct a slice and check for equality
        if (String_index(self, i) == String_index(other, 0) &&
            String_index(self, i + offset) == String_index(other, -1)) {
            index = (StringIndexT){i, i + other->length, 1};
            printf("%s", String_slice(self, index)->string);

            if (String_equals(String_slice(self, index), other)) return index;
        }
    }

    return not_found;
}

/// Check if `other` string is contained within the specified range in the original string
/// Time complexity is O(n*m) where n is the length of the string and m is the length of
/// the sub_string, when the length of `index` is smaller than the length of the `other`
/// string.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *sub_string = String_from("World");
/// String_contains_in_range(string, sub_string, (StringIndexT){0, 5, 1}); // false
/// String_contains_in_range(string, sub_string, (StringIndexT){0, 12, 1}); // true
/// ```
StringIndexT
String_contains_in_range(const StringT *self, const StringT *other, StringIndexT index) {
    StringIndexT not_found = (StringIndexT){0, 0, 1};
    StringIndexT slice_index;
    ssize_t offset = other->length - 1;

    if (index.step != 1) {
        fprintf(stderr, "String_contains_in_range: step must be 1");
        exit(EXIT_FAILURE);
    }

    if (index.stop - index.start < other->length) return not_found;

    for (ssize_t i = index.start; i < index.stop - offset; ++i) {
        // Checking if the first and last characters match
        // If so, then we construct a slice and check for equality
        if (String_index(self, i) == String_index(other, 0) &&
            String_index(self, i + offset) == String_index(other, -1)) {
            slice_index = (StringIndexT){i, i + other->length, 1};

            if (String_equals(String_slice(self, slice_index), other)) return slice_index;
        }
    }
    return not_found;
}

/// Replace all occurrences of a sub_string with a replacement string recursively.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *sub_string = String_from("World");
/// StringT *replacement = String_from("Universe");
/// String_replace(string, sub_string, replacement); // "Hello, Universe!"
/// ```
StringT *
String_replace(const StringT *self, const StringT *sub_string,
               const StringT *replacement) {
    StringIteratorT *iterator = String_split(self, sub_string);
    return String_join(iterator, replacement);
}

/// Split the string at the provided delimiter a limited number of times.
///
/// # Example
/// ```c
/// StringT *string = String_from("Apple, Banana, Mango, Orange");
/// StringIteratorT *iterator = String_split_limit(string, String_from(", "), 2);
///
/// StringIterator_next(iterator); // "Apple"
/// StringIterator_next(iterator); // "Banana"
/// StringIterator_next(iterator); // "Mango, Orange"
/// ```
StringIteratorT *
String_split_with_limit(const StringT *self, const StringT *delimiter, ssize_t limit) {
    StringIteratorT *iterator = StringIterator_new();
    StringIndexT index;
    StringIndexT slice_index;
    ssize_t start = 0;

    // Special case for `limit`
    // If limit is -1, then we iterate until the string is exhausted
    if (!limit) {
        StringIterator_append(iterator, self);
        return iterator;
    } else if (limit == -1) {
        limit = self->length;
    } else if (limit < -1) {
        fprintf(stderr, "String_split_limit: limit must be greater than -1");
        exit(EXIT_FAILURE);
    }

    index = String_contains(self, delimiter);

    // Get the index of the delimiter and slice the string from that index
    // then append it to the list, then set the start value to current stop value
    // and continue unit the delimiter is not found or `limit` is exhausted
    while (index.stop && limit--) {
        slice_index = StringIndex_new(start, index.start, 1);
        StringIterator_append(iterator, String_slice(self, slice_index));
        start = index.stop;
        index = String_contains_in_range(self, delimiter,
                                         StringIndex_new(start, self->length, 1));
    }

    StringIterator_append(iterator,
                          String_slice(self, StringIndex_new(start, self->length, 1)));

    return iterator;
}

/// Split the string at the provided delimiter.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringIteratorT *string_iterator = String_split(string, String_from(", "));
///
/// StringIterator_next(string_iterator); // "Hello"
/// StringIterator_next(string_iterator); // "World"
/// ```
StringIteratorT *
String_split(const StringT *self, const StringT *delimiter) {
    return String_split_with_limit(self, delimiter, -1);
}

/// Split the string based on newline chars.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello,\nWorld");
/// StringIteratorT *string_iterator = String_split_lines(string);
///
/// StringIterator_next(string_iterator); // "Hello,"
/// StringIterator_next(string_iterator); // "World"
/// ```
StringIteratorT *
String_split_lines(const StringT *self) {
    return String_split(self, String_from("\n"));
}

/// Split the string based on whitespace characters.
/// Whitespace charters include:
///  - space           ' '
///  - tab             '\t'
///  - newline         '\n'
///  - vertical tab    '\v'
///  - form feed       '\f'
///  - carriage return '\r'
///
/// # Example
/// ```c
/// StringT *string = String_from("Apple Banana Mango\n        \
///                                Cauliflower Broccoli Cabbage");
/// StringIteratorT *string_iterator = String_split_whitespace(string);
/// StringT *word;
///
/// while (word = StringIterator_next(string_iterator))
///     printf("%s\n", word);
///
/// ```
///
/// ## Output
/// ```
/// Apple
/// Banana
/// Mango
/// Cauliflower
/// Broccoli
/// Cabbage
/// ```
StringIteratorT *
String_split_whitespace(const StringT *self) {
    char ch;
    StringIteratorT *iterator = StringIterator_new();
    StringT *string = String_new(0);

    for (int i = 0; i < self->length; ++i) {
        if (CHAR_IS_WHITESPACE(self->string[i])) {
            if (string->length) StringIterator_append(iterator, string);
            string = String_new(0);
        } else
            String_push(string, self->string[i]);
    }
    if (string->length) StringIterator_append(iterator, string);

    return iterator;
}

/// Join the strings in the iterator with the provided delimiter.
/// The delimiter is not added to the end of the joined string.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringIteratorT *string_iterator = String_split(string, String_from(", "));
/// StringT *joined_string = String_join(string_iterator, String_from(" "));
/// printf("%s", joined_string->string); // Hello World
/// ```
StringT *
String_join(StringIteratorT *self, const StringT *delimiter) {
    StringT *string = String_new(0);
    ssize_t iterator_length = self->length;


    for (ssize_t i = 0; i < iterator_length - 1; ++i) {
        String_concatenate_inplace(string, StringIterator_next(self));
        String_concatenate_inplace(string, delimiter);
    }

    String_concatenate_inplace(string, StringIterator_next(self));

    return string;
}

/// Helper function to check if the substring of the `self` matches the string `other`
static bool
String_check_equals_in_range(const StringT *self, const StringT *other,
                             StringIndexT index) {
    if (index.stop > self->length) {
        fprintf(stderr, "String_check_equals_in_range: index out of range");
        exit(EXIT_FAILURE);
    }

    if (other->length > self->length) return false;

    for (ssize_t i = index.start; i < index.stop; i += index.step)
        if (self->string[i] != other->string[i - index.start]) return false;

    return true;
}

/// Check if the string starts with the provided prefix.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *prefix = String_from("Hello");
/// printf("%d", String_starts_with(string, prefix)); // 1
/// ```
bool
String_starts_with(const StringT *self, const StringT *prefix) {
    return String_check_equals_in_range(self, prefix,
                                        StringIndex_new(0, prefix->length, 1));
}

/// Check if the string ends with the provided suffix.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *suffix = String_from("World");
/// printf("%d", String_ends_with(string, suffix)); // 1
/// ```
bool
String_ends_with(const StringT *self, const StringT *suffix) {
    return String_check_equals_in_range(
        self, suffix, StringIndex_new(self->length - suffix->length, self->length, 1));
}

/// Reverse the string.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *reversed_string = String_reverse(string);
/// printf("%s", reversed_string->string); // dlroW ,olleH
/// ```
StringT *
String_reverse(const StringT *self) {
    return String_slice(self, StringIndex_new(self->length - 1, -1, -1));
}

/// Convert the string to uppercase.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *upper_string = String_to_upper(string);
/// printf("%s", upper_string->string); // HELLO, WORLD
/// ```
StringT *
String_to_upper(const StringT *self) {
    StringT *new_string = String_copy(self);

    for (ssize_t i = 0; i < new_string->length; ++i)
        CHAR_TO_UPPERCASE(new_string->string[i]);


    return new_string;
}

/// Convert the string to lowercase.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *lower_string = String_to_lower(string);
/// printf("%s", lower_string->string); // hello, world
/// ```
StringT *
String_to_lower(const StringT *self) {
    StringT *new_string = String_copy(self);

    for (ssize_t i = 0; i < new_string->length; ++i)
        CHAR_TO_LOWERCASE(new_string->string[i]);

    return new_string;
}

/// Convert the string to Title case.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *lower_string = String_to_lower(string);
/// printf("%s", lower_string->string); // hello, world
/// ```
StringT *
String_to_title(const StringT *self) {
    StringT *new_string = String_copy(self);
    char ch;

    if (!new_string->length) return new_string;

    CHAR_TO_UPPERCASE(new_string->string[0]);

    // Capitalizing the character that comes after a space char.
    for (int i = 1; i < new_string->length - 1; ++i) {
        ch = new_string->string[i];
        if (CHAR_IS_WHITESPACE(ch)) CHAR_TO_UPPERCASE(new_string->string[i + 1]);
    }

    return new_string;
}

/// Capitalize the string, i.e. make the first character uppercase.
/// Has time complexity of O(1).
///
/// # Example
/// ```c
/// StringT *string = String_from("hello, world");
/// StringT *capitalized_string = String_to_capitalize(string);
/// printf("%s", capitalized_string->string); // `Hello, world`
/// ```
StringT *
String_to_capitalize(const StringT *self) {
    StringT *new_string = String_copy(self);

    if (!new_string->length) return new_string;

    CHAR_TO_UPPERCASE(new_string->string[0]);

    return new_string;
}

/// Swap the case of the string.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *swapped_string = String_swap_case(string);
/// printf("%s", swapped_string->string); // `hELLO, wORLD`
/// ```
StringT *
String_swap_case(const StringT *self) {
    StringT *new_string = String_copy(self);
    char ch;

    for (ssize_t i = 0; i < new_string->length; ++i)
        CHAR_SWAP_CASE(new_string->string[i]);

    return new_string;
}

/// Check if the string is alphanumeric.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// printf("%d", String_is_alphanumeric(string)); // `0`
/// ```
bool
String_is_alphanumeric(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_ALPHA_NUMERIC(self->string[i])) return false;

    return true;
}

/// Check if the string is alphabetic.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello World");
/// printf("%d", String_is_alphabetic(string)); // `1`
/// ```
bool
String_is_alphabetic(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_ALPHABET(self->string[i])) return false;

    return true;
}

/// Check if the string is uppercase.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("HELLO WORLD");
/// printf("%d", String_is_uppercase(string)); // `1`
/// ```
bool
String_is_uppercase(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_UPPERCASE(self->string[i])) return false;

    return true;
}

/// Check if the string is lowercase.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("hello world");
/// printf("%d", String_is_uppercase(string)); // `1`
/// ```
bool
String_is_lowercase(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_LOWERCASE(self->string[i])) return false;

    return true;
}

/// Check if the string is numeric.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("12345");
/// printf("%d", String_is_numeric(string)); // `1`
/// ```
bool
String_is_numeric(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_DIGIT(self->string[i])) return false;

    return true;
}

/// Check if the string is decimal.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("123.45");
/// printf("%d", String_is_decimal(string)); // `1`
/// ```
bool
String_is_decimal(const StringT *self) {
    bool decimal_found = false;

    for (ssize_t i = 0; i < self->length; ++i) {
        if (self->string[i] == '.') {
            if (decimal_found) return false;
            decimal_found = true;
        } else if (!CHAR_IS_DIGIT(self->string[i]))
            return false;
    }

    return true;
}

/// Check if the string is whitespace.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from(" ");
/// printf("%d", String_is_whitespace(string)); // `1`
/// ```
bool
String_is_whitespace(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_WHITESPACE(self->string[i])) return false;

    return true;
}

/// Trim whitespace from left and right of the string.
/// Has worst case time complexity of O(n)
///
/// # Example
/// ```c
/// StringT *string = String_from("  Hello, World ");
/// printf("%s", String_trim_whitespace(string)->string); // `Hello, World`
/// ```
StringT *
String_trim_whitespace(const StringT *self) {
    return String_trim_left(String_trim_right(self));
}

/// Trim whitespace from left of the string.
/// Has worst case time complexity of O(n)
///
/// # Example
/// ```c
/// StringT *string = String_from("  Hello, World ");
/// printf("%s", String_trim_whitespace(string)->string); // `Hello, World `
/// ```
StringT *
String_trim_left(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_WHITESPACE(self->string[i]))
            return String_slice(self, StringIndex_new(i, self->length, 1));
    return String_copy(self);
}

/// Trim whitespace right left of the string.
/// Has worst case time complexity of O(n)
///
/// # Example
/// ```c
/// StringT *string = String_from("  Hello, World ");
/// printf("%s", String_trim_whitespace(string)->string); // `  Hello, World`
/// ```
StringT *
String_trim_right(const StringT *self) {
    for (ssize_t i = self->length - 1; i >= 0; ++i)
        if (!CHAR_IS_WHITESPACE(self->string[i]))
            return String_slice(self, StringIndex_new(0, self->length - i, 1));
    return String_copy(self);
}

/// Centre the text within the given width.
/// Padding is done with space chars
///
/// # Example
/// ```c
/// StringT *string = String_from("Hi");
/// printf("%s", String_centre(string, 4)); ` Hi `
/// ```
StringT *
String_centre(const StringT *self, ssize_t width) {
    ssize_t left_shift_width = (self->length - width) / 2;
    ssize_t right_shift_width = self->length - left_shift_width;
    return String_right_justify(String_left_justify(self, left_shift_width),
                                right_shift_width);
}

/// Justify the text towards the left within the given width.
/// Padding is done with space chars
///
/// # Example
/// ```c
/// StringT *string = String_from("Hi");
/// printf("%s", String_left_justify(string, 4)); `Hi  `
/// ```
StringT *
String_left_justify(const StringT *self, ssize_t width) {
    ssize_t length_to_fill = width - self->length;

    if (length_to_fill < 0) return String_copy(self);

    // Concatenate the string with `length_to_fill` number of whitespaces
    return String_concatenate(String_copy(self),
                              String_repeat(String_from(" "), length_to_fill));
}

/// Justify the text towards the right within the given width.
/// Padding is done with space chars
///
/// # Example
/// ```c
/// StringT *string = String_from("Hi");
/// printf("%s", String_right_justify(string, 4)); `  Hi`
/// ```
StringT *
String_right_justify(const StringT *self, ssize_t width) {
    ssize_t length_to_fill = width - self->length;

    if (length_to_fill < 0) return String_copy(self);

    // Concatenate `length_to_fill` number of whitespaces with the string.
    return String_concatenate(String_repeat(String_from(" "), length_to_fill), self);
}

/// Split the string into chunks of given size.
/// Has time complexity of O(n)
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringIteratorT *iterator = String_chunks(string, 3);
/// for (StringT *chunk = StringIterator_next(iterator); chunk != NULL; chunk =
/// StringIterator_next(iterator))
/// printf("%s", chunk->string);
///    // `Hel`
///    // `lo,`
///    // ` Wo`
///    // `rld`
/// ```
StringIteratorT *
Stirng_chunks(const StringT *self, ssize_t chunk_size) {
    StringIteratorT *iterator = StringIterator_new();

    for (ssize_t i = 0; i < self->length; i += chunk_size)
        StringIterator_append(iterator,
                              String_slice(self, StringIndex_new(i, i + chunk_size, 1)));

    return iterator;
}

StringT *String_format(const StringT *self, ...);

typedef struct {
    const StringT *unformatted_string;
    StringIndexT *iterator;
} TokenT;

typedef enum {
    FORMAT_EXPRESSION_START = '{',
    FORMAT_EXPRESSION_STOP = '}',

    FORMAT_EXPRESSION_NOT_FOUND = 0xFFFF,
    FORMAT_EXPRESSION_NOT_CLOSED = 0xEEE,
} Tokens;

void
Token_get_next(TokenT *self) {
    int is_curly_brace_open = 0;

    for (ssize_t i = self->iterator->start; i < self->unformatted_string->length; ++i) {
        // printf("%c %ld", self->unformatted_string->string[i], i);
        switch (self->unformatted_string->string[i]) {
            case FORMAT_EXPRESSION_START:
                self->iterator->start = i + 1;

                is_curly_brace_open++;
                break;
            case FORMAT_EXPRESSION_STOP:
                is_curly_brace_open--;

                if (is_curly_brace_open != 0) {
                    fprintf(stderr, "%ld: Format expression is not open", i);
                    exit(FORMAT_EXPRESSION_NOT_CLOSED);
                }
                self->iterator->stop = i;

                return;
        }
    }

    self = NULL;
}

TokenT *
Token_new(const StringT *self) {
    TokenT *token = malloc(sizeof *token);
    StringIndexT *iterator = malloc(sizeof *iterator);
    *iterator = StringIndex_new(0, 0, 1);
    *token = (TokenT){self, iterator};

    Token_get_next(token);
    return token;
}

void
Token_free(TokenT *self) {
    free(self->iterator);
    free(self);
}

typedef enum {
    STRING,
    INTEGER,
    FLOAT,
    DOUBLE,
    CHAR,
    BOOLEAN,
    POINTER,
    ARRAY,
    STRUCT,
    UNION,
    ENUM,
    VOID,
    UNKNOWN,
} FormatTypes;

typedef struct {
    FormatTypes type;
    StringT *options;
} FormatExpressionT;

FormatExpressionT *
Format_new(TokenT *token) {
    FormatExpressionT *format = malloc(sizeof *format);
    printf("%s\n", String_slice(token->unformatted_string, *token->iterator)->string);
    StringIteratorT *split = String_split(
        String_slice(token->unformatted_string, *token->iterator), String_from(":"));
    StringT *type_str = StringIterator_next(split);
    StringT *options = StringIterator_next(split);
    printf("%zd\n", split->length);
    FormatTypes type;

    switch (type_str->string[0]) {
        case 's':
            type = STRING;
            break;
        case 'i':
            type = INTEGER;
            break;
        case 'f':
            type = FLOAT;
            break;
        case 'd':
            type = DOUBLE;
            break;
        case 'c':
            type = CHAR;
            break;
        case 'b':
            type = BOOLEAN;
            break;
        default:
            type = UNKNOWN;
    }

    StringIterator_free(split);
    *format = (FormatExpressionT){.type = type, .options = options};

    return format;
}

void
Format_free(FormatExpressionT *self) {
    if (self->options != NULL) String_free(self->options);
    free(self);
}

// TODO: Fix the current formatting options and add extra
StringT *
Format_string(FormatExpressionT *self, const StringT *string) {
    StringT *formatted_string = String_new(0);

    for (ssize_t i = 0, current = i; i < self->options->length; ++i) {
        switch (self->options->string[i]) {
            case '^':
                printf("centre\n");
                while (self->options->string[i] != ',' && i < self->options->length) {
                    i++;
                }
                formatted_string =
                    String_centre(string, atoi(self->options->string + current));
        }
    }

    return formatted_string;
}
