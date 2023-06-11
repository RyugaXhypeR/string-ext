#include "../include/string.h"

#include "../include/dbg.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#define WHITESPACE_CHARS " \t\n\r"
#define CHAR_IS_WHITESPACE(ch)                                                           \
    ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')
#define CHAR_IS_DIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define CHAR_IS_ALPHABET(ch)                                                             \
    (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))
#define CHAR_IS_LOWERCASE(ch) (((ch) >= 'a' && (ch) <= 'z') || !CHAR_IS_ALPHABET(ch))
#define CHAR_IS_UPPERCASE(ch) (((ch) >= 'A' && (ch) <= 'Z') || !CHAR_IS_ALPHABET(ch))
#define CHAR_IS_ALPHANUMERIC(ch) ((CHAR_IS_ALPHABET(ch)) || (CHAR_IS_DIGIT(ch)))

#define CHAR_TO_LOWERCASE(ch)                                                            \
    if (CHAR_IS_ALPHABET(ch)) (ch |= 0x20)
#define CHAR_TO_UPPERCASE(ch)                                                            \
    if (CHAR_IS_ALPHABET(ch)) (ch &= ~0x20)
#define CHAR_SWAP_CASE(ch)                                                               \
    if (CHAR_IS_ALPHABET(ch)) (ch ^= 0x20)


#define U8_MAX 256
#define MAX_2(a, b) ((a > b) ? (a) : (b))

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
/// Meant to be used to iterate over multiple `const StringT` objects.
StringIteratorT *
StringIterator_new() {
    StringIteratorT *self = malloc(sizeof *self);
    const StringT **string_array = malloc(sizeof **string_array);

    if (self == NULL) ERR("Unable to allocate memory for `StringIteratorT`");
    if (string_array == NULL) ERR("Unable to allocate memory for `StringT` array");

    *self = (StringIteratorT){
        .strings = string_array, .index = 0, .length = 0, .allocated = 1};
    return self;
}

const StringT *
StringIterator_next(StringIteratorT *self) {
    if (self->index >= self->length) return NULL;
    return self->strings[self->index++];
}

const StringT *
StringIterator_get(StringIteratorT *self) {
    return *self->strings;
}

void
StringIterator_free(StringIteratorT *self) {
    free(self->strings);
    free(self);
}

void
StringIterator_append(StringIteratorT *self, const StringT *string) {
    if (self->length >= self->allocated) {
        self->allocated <<= 1;
        self->strings = realloc(self->strings, self->allocated * sizeof *self->strings);
    }

    if (self->strings) ERR("Unable to reallocate memory for strings");

    self->strings[self->length++] = string;
}

/* ------------------------------ StringIndexT ------------------------------ */
StringIndexT
StringIndex__init__(size_t nargs, ...) {
    va_list args;
    va_start(args, nargs);

    ssize_t start, stop, step;

    switch (nargs) {
        case 1:
            start = 0;
            stop = va_arg(args, ssize_t);
            step = 1;
            break;
        case 2:
            start = va_arg(args, ssize_t);
            stop = va_arg(args, ssize_t);
            step = 1;
            break;
        case 3:
            start = va_arg(args, ssize_t);
            stop = va_arg(args, ssize_t);
            step = va_arg(args, ssize_t);
            break;
        default:
            fprintf(stderr, "Invalid number of arguments");
            exit(EXIT_FAILURE);
    }

    va_end(args);

    return StringIndex_new(start, stop, step);
}

StringIndexT
StringIndex_new(ssize_t start, ssize_t stop, ssize_t step) {
    if (step == 0) {
        fprintf(stderr, "Step cannot be 0");
        exit(EXIT_FAILURE);
    }
    return (StringIndexT){.start = start, .stop = stop, .step = step};
}

ssize_t
StringIndex_len(StringIndexT self) {
    return (self.stop - self.start) / self.step;
}

/* ------------------------------ StringT ------------------------------ */

/// Re-allocate extra space for the string.
static void
String_re_allocate(StringT *self, ssize_t new_size) {
    ssize_t new_allocated;

    if (new_size <= self->allocated) return;

    new_allocated = (new_size + (new_size >> 3) + 6) & ~3;

    DBG("Re-allocating string from %ld to %ld", self->allocated, new_allocated);
    self->string = realloc(self->string, new_allocated * sizeof *self->string);
    self->allocated = new_allocated;
}

StringT *
String_pre_allocated(char *str, ssize_t size) {
    StringT *string = malloc(size * (sizeof *string));

    if (string == NULL) ERR("Unable to allocate memory for `StringT`");

    *string = (StringT){.string = str, .length = c_string_length(str), .allocated = size};

    return string;
}

static void
String_push(StringT *self, char ch) {
    String_re_allocate(self, self->length + 1);

    self->string[self->length] = ch;
    self->length++;
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
    char *string = malloc(size * (sizeof *string));

    if (self == NULL) ERR("Unable to allocate memory for `StringT`");
    if (string == NULL) ERR("Unable to allocate memory for `char *`");

    *self = (StringT){.string = string, .length = 0, .allocated = size};
    return self;
}

/// Create StrngT object from string array of given length.
static StringT *
String_from_char_array_with_length(const char *string, ssize_t length) {
    StringT *self = malloc(sizeof *self);
    char *new_string = malloc((length + 1) * sizeof *new_string);

    if (self == NULL) ERR("Unable to allocate memory for `StringT`");
    if (new_string == NULL) ERR("Unable to allocate memory for `char *`");

    for (ssize_t i = 0; i < length; ++i) new_string[i] = string[i];
    new_string[length] = '\0';

    *self = (StringT){.string = new_string, .length = length, .allocated = length};

    return self;
}

/// Copy the `StringT` object.
StringT *
String_copy(const StringT *self) {
    return String_from(self->string);
}

/// Deep free the `StringT` object and the inner C-string.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// string->string // Hello, World
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
/// char c = String_index(string, 0);  // 'H'
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
/// StringT *slice = String_slice(string, 0, 5, 1);   // `Hello`
/// StringT *slice = String_slice(string, 5, -1, -1); // `olleH`
/// StringT *slice = String_slice(string, 0, 5, 2);   // `Hlo`
/// ``
StringT *
String_slice(const StringT *self, StringIndexT index) {
    StringT *slice = String_new(0);

    // Calculate the length of this range and iterate that many times
    int slice_length = StringIndex_len(index);

    while (slice_length--) {
        String_push(slice, self->string[index.start]);
        index.start += index.step;
    }

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

    for (ssize_t i = 0; i < other->length; ++i)
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
    ssize_t new_length;
    StringT *new_string;

    if (times < 0) return String_new(0);

    new_length = self->length * times;
    new_string = String_pre_allocated(self->string, new_length);

    for (ssize_t i = 1; i < times; ++i) String_concatenate_inplace(new_string, self);

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

    for (ssize_t i = 0; i < self->length; ++i)
        if (self->string[i] != other->string[i]) return false;

    return true;
}

/// Construct the bad match table, which holds the relative positions of the letters wrt
/// to their previous positions. For string "ABCBABA", the table would look like
/// {A: 1, B: 1, C: 4}, these are just stored in a large array of size `U8_MAX` instead of
/// preallocated hash_table.
/// TODO: use smart hashing to only allocate for chars that have a possiblilty of occuring
/// in strings other then all 256 ASCII chars.
static void
_construct_bad_match_table(const StringT *self, ssize_t *match_table) {
    for (ssize_t i = 0; i < U8_MAX; i++) {
        match_table[i] = 0;
    }

    for (ssize_t i = 0; i < self->length; i++) {
        match_table[self->string[i]] = MAX_2(1, self->length - i - 1);
    }
}

/// Compare the string and substring from a starting point in reverse.
static bool
_reverse_string_compare_from_starting_point(const StringT *self, const StringT *sub,
                                            size_t start) {
    ssize_t j = sub->length - 1;
    for (ssize_t i = start; j >= 0 && self->string[i] == sub->string[j]; i--, j--)
        ;
    return j < 0;
}

/// Check if the string contains a given string.
/// Time complexity is O(n*m) where n is the length of the string and m is the length of
/// the sub_string, when the `other` is smaller than `self`.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *sub_string = String_from("World");
/// String_contains(string, sub_string); // StringIndex(7, 12)
/// ```
StringIndexT
String_contains(const StringT *self, const StringT *other) {
    return String_contains_in_range(self, other, StringIndex(self->length));
}

/// Check if `other` string is contained within the specified range in the original string
/// Time complexity is O(n*m) where n is the length of the string and m is the length of
/// the sub_string, when the length of `index` is smaller than the length of the `other`
/// string.
/// consume less memory and time.
///
/// Implemented using Boyer-Moore algorithm.
/// For more information checkout:
/// https://en.wikipedia.org/wiki/Boyer–Moore_string-search_algorithm
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *sub_string = String_from("World");
/// String_contains_in_range(string, sub_string,
///                          StringIndex(0, 5)); // StringIndex(0, 0, 1)
/// String_contains_in_range(string, sub_string,
///                          StringIndex(0, 12)); // StringIndex(7, 12)
/// ```
StringIndexT
String_contains_in_range(const StringT *self, const StringT *other, StringIndexT index) {
    StringIndexT not_found = StringIndex(0, 0, 1);
    ssize_t match_table[U8_MAX];
    ssize_t shift;

    _construct_bad_match_table(other, match_table);

    if (index.step != 1) {
        fprintf(stderr, "String_contains_in_range: step must be 1");
        exit(EXIT_FAILURE);
    }

    for (ssize_t i = index.start + other->length - 1; i < index.stop;) {
        shift = match_table[self->string[i]];

        if (!shift)
            shift = other->length;
        else if (_reverse_string_compare_from_starting_point(self, other, i))
            return StringIndex(i - other->length + 1, i + 1);

        i += shift;
    }

    if (index.stop - index.start < other->length) return not_found;

    return not_found;
}

/// Check if the string contains the given char.
/// Time complexity is O(n) where n is the length of the string.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// String_contains_char(string, 'W'); // StringIndex(7, 8)
/// ```
StringIndexT
String_contains_char(const StringT *self, const char character) {
    return String_contains_char_in_range(self, character, StringIndex(self->length));
}

/// Check if the string contains the given char in the specified range.
/// Time complexity is O(n) where n is the length of the string.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// String_contains_char_in_range(string, 'W', StringIndex(0, 5)); // StringIndex(0,
/// 0, 1) String_contains_char_in_range(string, 'W', StringIndex(0, 12)); //
/// StringIndex(7, 8)
/// ```
StringIndexT
String_contains_char_in_range(const StringT *self, const char character,
                              StringIndexT index) {
    for (ssize_t i = index.start; i < index.stop; i += index.step)
        if (self->string[i] == character) return StringIndex(i, i + 1);
    return StringIndex(0, 0, 1);
}

/// Check if the string contains any char from the char class.
/// Similar to regex: `[...]`
/// Time complexity is O(n*m) where n is the length of the string and m is the length
/// of the char class.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *char_class = String_from("Wd");
/// String_contains_char_class(string, char_class); // StringIndex(7, 8)
/// ```
StringIndexT
String_find_from_char_class(const StringT *self, const StringT *characters) {
    return String_find_from_char_class_in_range(self, characters,
                                                StringIndex(self->length));
}

/// Check if the string contains any char from the char class in the specified range.
/// Similar to regex: `[...]`
/// Time complexity is O(n*m) where n is the length of the string and m is the length
/// of the char class.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World!");
/// StringT *char_class = String_from("Wd");
/// String_contains_char_class_in_range(string, char_class,
///                                     StringIndex(0, 5)); // StringIndex(0, 0, 1)
/// String_contains_char_class_in_range(string, char_class,
///                                     StringIndex(8, 12)); // StringIndex(11, 12)
/// ```
StringIndexT
String_find_from_char_class_in_range(const StringT *self, const StringT *characters,
                                     StringIndexT index) {
    StringIndexT not_found = StringIndex(0, 0, 1);
    StringIndexT slice_index;
    ssize_t index_length = StringIndex_len(index);

    for (ssize_t i = index.start; i < index.stop && index_length--; ++i) {
        slice_index = String_contains_char(characters, self->string[i]);
        if (!StringIndex_len(slice_index)) continue;
        return slice_index;
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
String_split_limit(const StringT *self, const StringT *delimiter, ssize_t limit) {
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
        slice_index = StringIndex(start, index.start);
        StringIterator_append(iterator, String_slice(self, slice_index));
        start = index.stop;
        index =
            String_contains_in_range(self, delimiter, StringIndex(start, self->length));
    }

    StringIterator_append(iterator, String_slice(self, StringIndex(start, self->length)));

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
    return String_split_limit(self, delimiter, -1);
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
/// StringT *string = String_from("Apple       Banana   Mango\n"
///                               "Cauliflower Broccoli Cabbage");
/// StringIteratorT *string_iterator = String_split_whitespace(string);
/// StringT *word;
///
/// while (word = StringIterator_next(string_iterator))
///     word->string; // "Apple", "Banana", "Mango", "Cauliflower", "Broccoli",
///     "Cabbage"
/// ```
StringIteratorT *
String_split_whitespace(const StringT *self) {
    return String_split_whitespace_limit(self, -1);
}

StringIteratorT *
String_split_lines_limit(const StringT *self, ssize_t limit) {
    return String_split_limit(self, String_from("\n"), limit);
}

StringIteratorT *
String_split_whitespace_limit(const StringT *self, ssize_t limit) {
    StringIteratorT *iterator = StringIterator_new();
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

    for (ssize_t i = 0; i < self->length && limit--; ++i) {
        if (CHAR_IS_WHITESPACE(self->string[i])) {
            StringIterator_append(iterator, String_slice(self, StringIndex(start, i)));
            while (CHAR_IS_WHITESPACE(self->string[i]) && i < self->length) ++i;
            start = i;
        }
    }


    return iterator;
}

StringIteratorT *
String_split_in_range(const StringT *self, const StringT *delimiter, StringIndexT index) {
    return String_split(String_slice(self, index), delimiter);
}

StringIteratorT *
String_right_split(const StringT *self, const StringT *delimiter) {
    return String_right_split_limit(self, delimiter, -1);
}

StringIteratorT *
String_right_split_limit(const StringT *self, const StringT *delimiter, ssize_t limit) {
    StringIteratorT *iterator = StringIterator_new();
    StringIndexT index;
    ssize_t start = self->length;

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
    for (ssize_t i = 0; i < self->length; ++i) {
        if (index.stop) {
            if (start != index.start) {
                StringIterator_append(
                    iterator, String_slice(self, StringIndex(index.start, start)));
                if (!--limit) break;
            }
            start = index.start - 1;
        }
        index =
            String_contains_in_range(self, delimiter, StringIndex(0, index.start - 1));
    }

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
/// joined_string->string // Hello World
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
/// String_starts_with(string, prefix) // true
/// ```
bool
String_starts_with(const StringT *self, const StringT *prefix) {
    return String_check_equals_in_range(self, prefix, StringIndex(prefix->length));
}

/// Check if the string ends with the provided suffix.
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *suffix = String_from("World");
/// String_ends_with(string, suffix) // true
/// ```
bool
String_ends_with(const StringT *self, const StringT *suffix) {
    return String_check_equals_in_range(
        self, suffix, StringIndex(self->length - suffix->length, self->length));
}

/// Reverse the string.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *reversed_string = String_reverse(string);
/// reversed_string->string // dlroW ,olleH
/// ```
StringT *
String_reverse(const StringT *self) {
    return String_slice(self, StringIndex(self->length - 1, -1, -1));
}

/// Convert the string to uppercase.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// StringT *upper_string = String_to_upper(string);
/// upper_string->string // HELLO, WORLD
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
/// lower_string->string // hello, world
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
/// lower_string->string // hello, world
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
/// StringT *capitalized_string = String_to_capital(string);
/// capitalized_string->string // Hello, world
/// ```
StringT *
String_to_capital(const StringT *self) {
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
/// swapped_string->string // hELLO, wORLD
/// ```
StringT *
String_swap_case(const StringT *self) {
    StringT *new_string = String_copy(self);

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
/// String_is_alphanumeric(string) // true
/// ```
bool
String_is_alphanumeric(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_ALPHANUMERIC(self->string[i])) return false;

    return true;
}

/// Check if the string is alphabetic.
/// Has time complexity of O(n).
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello World");
/// String_is_alphabetic(string) // false
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
/// String_is_uppercase(string) // true
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
/// String_is_lowercase(string) // false
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
/// String_is_numeric(string) // true
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
/// String_is_decimal(string) // true
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
/// String_is_whitespace(string) // true
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
/// String_trim_whitespace(string)->string // Hello, World
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
/// String_trim_whitespace(string)->string // `Hello, World `
/// ```
StringT *
String_trim_left(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_WHITESPACE(self->string[i]))
            return String_slice(self, StringIndex(i, self->length));
    return String_copy(self);
}

/// Trim whitespace right left of the string.
/// Has worst case time complexity of O(n)
///
/// # Example
/// ```c
/// StringT *string = String_from("  Hello, World ");
/// String_trim_whitespace(string)->string // `  Hello, World`
/// ```
StringT *
String_trim_right(const StringT *self) {
    for (ssize_t i = self->length - 1; i >= 0; --i)
        if (!CHAR_IS_WHITESPACE(self->string[i]))
            return String_slice(self, StringIndex(i + 1));
    return String_copy(self);
}

StringT *
String_pad(const StringT *self, ssize_t left_pad, ssize_t right_pad) {
    StringT *left_padded =
        String_concatenate(String_repeat(String_from(" "), left_pad), self);
    StringT *right_padded =
        String_concatenate(left_padded, String_repeat(String_from(" "), right_pad));

    return right_padded;
}

/// Centre the text within the given width.
/// Padding is done with space chars
///
/// # Example
/// ```c
/// StringT *string = String_from("Hi");
/// String_centre(string, 4)->string // ` Hi `
/// ```
StringT *
String_centre(const StringT *self, ssize_t width) {
    ssize_t margin, left_pad;

    if (self->length >= width) return String_copy(self);

    margin = width - self->length;
    left_pad = margin / 2;

    return String_pad(self, left_pad, margin - left_pad);
}

/// Justify the text towards the left within the given width.
/// Padding is done with space chars
///
/// # Example
/// ```c
/// StringT *string = String_from("Hi");
/// String_left_justify(string, 4) // `Hi  `
/// ```
StringT *
String_left_justify(const StringT *self, ssize_t width) {
    ssize_t length_to_fill = width - self->length;

    if (length_to_fill < 0) return String_copy(self);

    // Concatenate the string with `length_to_fill` number of whitespaces
    return String_concatenate(self, String_repeat(String_from(" "), length_to_fill));
}

/// Justify the text towards the right within the given width.
/// Padding is done with space chars
///
/// # Example
/// ```c
/// StringT *string = String_from("Hi");
/// String_right_justify(string, 4) // `  Hi`
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
///
/// while (StringIterator_next(iterator))
///    StringIterator_get(iterator); // `Hel`, `lo,`, ` Wo`, `rld`
/// ```
StringIteratorT *
Stirng_chunks(const StringT *self, ssize_t chunk_size) {
    StringIteratorT *iterator = StringIterator_new();

    for (ssize_t i = 0; i < self->length; i += chunk_size)
        StringIterator_append(iterator,
                              String_slice(self, StringIndex(i, i + chunk_size)));

    return iterator;
}

/// Count the number of times the substring occurs in the string.
/// Has time complexity of O(n)
///
/// # Example
/// ```c
/// StringT *string = String_from("Hello, World");
/// String_count(string, String_from("l")) // 3
/// String_count(string, String_from("ll")) // 1
/// ```
ssize_t
String_count(const StringT *self, const StringT *sub_string) {
    ssize_t count = 0;
    StringIndexT contains = String_contains(self, sub_string);

    if (!contains.stop) return 0;

    while (contains.stop) {
        contains = String_contains_in_range(self, sub_string,
                                            StringIndex(contains.stop, self->length));
        count++;
    }

    return count;
}
