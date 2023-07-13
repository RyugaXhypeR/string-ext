#include "string_ext.h"

#include "string_dbg.h"

#include <stdarg.h> /* va_list, va_start, va_arg, va_end */
#include <stdlib.h> /* malloc, realloc */


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

/**
 * Convert negative index to positive index. If the index is positive,
 * it does nothing.
 */
static ssize_t
negative_index_to_positive(ssize_t index, size_t length) {
    if (index >= 0) return index;

    index += length;
    if (index < 0) ERR("Index out of range");

    return index;
}

/**
 * Calculate the length of a NULL terminated C string1
 *
 * ..note:: The string should be NULL terminated!
 */
static size_t
c_string_length(const char *string) {
    size_t length = 0;

    while (*string++) length++;

    return length;
}

/* ------------------------------ StringIteratorT ------------------------------ */


/** Create and return a new ``StringIteratorT`` object. */
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

/**
 * Get the next value from the iterator.
 *
 * ..note:: The iterator will be incremented then the value will be returned.
 */
const StringT *
StringIterator_next(StringIteratorT *self) {
    if (self->index >= self->length) return NULL;
    return self->strings[self->index++];
}

/** Get the current value from the iterator. */
const StringT *
StringIterator_get(StringIteratorT *self) {
    return *self->strings;
}

/**
 * De-allocate memory stored for the iterator.
 *
 * ..note:: This function doesn't the ``StringT's``, it just frees the array.
 */
void
StringIterator_free(StringIteratorT *self) {
    free(self->strings);
    free(self);
}

/** Append a ``StringT`` to the end of the iterator. */
void
StringIterator_append(StringIteratorT *self, const StringT *string) {
    if (self->length >= self->allocated) {
        self->allocated <<= 1;
        self->strings = realloc(self->strings, self->allocated * sizeof *self->strings);
    }

    if (self->strings == NULL) ERR("Unable to reallocate memory for strings");

    self->strings[self->length++] = string;
}

/* ------------------------------ StringIndexT ------------------------------ */


/**
 * Helper function to construct a ``StringIndexT`` object by dynamically determining the
 * attribute values (start, stop, step).
 *
 * ..note:: This should only be used by ``StringIndex`` macro.
 */
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
            ERR("Invalid number of arguments");
    }

    va_end(args);

    return StringIndex_new(start, stop, step);
}

/** Create and return a new ``StringIndexT`` object, three parameters (start, stop, step)
 * passed in. */
StringIndexT
StringIndex_new(ssize_t start, ssize_t stop, ssize_t step) {
    if (step == 0) ERR("Step cannot be 0");

    return (StringIndexT){.start = start, .stop = stop, .step = step};
}

/**
 * Normalize the ``StringIndexT`` object by converting negative indices to positive
 * indices when step is positive.
 *
 * .. note::
 *    * This function doesn't check if the indices are out of range.
 *    * This function doesn't check if the step is 0.
 */
StringIndexT
StringIndex_normalize(StringIndexT self, ssize_t length) {
    if (self.stop < 0) {
        return self;
    }

    self.start = negative_index_to_positive(self.start, length);
    self.stop = negative_index_to_positive(self.stop, length);

    return self;
}

/** Check if the ``StringIndexT`` object is valid. */
bool
StringIndex_equal(StringIndexT self, StringIndexT other) {
    return self.start == other.start && self.stop == other.stop && self.step == other.step;
}

/** Calculate the length of the ``StringIndexT`` object. */
ssize_t
StringIndex_len(StringIndexT self) {
    return (self.stop - self.start) / self.step;
}

/* ------------------------------ StringT ------------------------------ */

/**
 * Create and return a new ``StringT`` object of given size.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_new(10);
 */
StringT *
String_new(ssize_t size) {
    StringT *self = malloc(sizeof *self);
    char *string = malloc(size * (sizeof *string));

    if (self == NULL) ERR("Unable to allocate memory for `StringT`");
    if (string == NULL) ERR("Unable to allocate memory for `char *`");

    *self = (StringT){.string = string, .length = 0, .allocated = size};
    return self;
}

/**
 * Create and return a new ``StringT`` object from a C string of given length.
 *
 * .. note:: Base string in the ``StringT`` will be NULL terminated.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from_char_array_with_length("Hello", 5);
 */
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

/**
 * Create and return a new ``StringT`` object from a C string.
 *
 * .. note:: Base string in the ``StringT`` will be NULL terminated.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello");
 */
StringT *
String_from(const char *_string) {
    return String_from_char_array_with_length(_string, c_string_length(_string));
}


/**
 * Internal function to re-allocate memory for the ``StringT`` object.
 *
 * .. note::
 *    * This function will only re-allocate memory if the new size is larger than the
 *      current allocated size.
 *    * If DEBUG is defined, this function will print a debug message.
 */
static void
String_re_allocate(StringT *self, ssize_t new_size) {
    ssize_t new_allocated;

    if (new_size <= self->allocated) return;

    new_allocated = (new_size + (new_size >> 3) + 6) & ~3;

    DBG("Re-allocating string from %ld to %ld", self->allocated, new_allocated);
    self->string = realloc(self->string, new_allocated * sizeof *self->string);
    self->allocated = new_allocated;
}

/**
 * Allocate exactly the amount of memory requested for the ``StringT`` object.
 *
 * .. note::
 *    * This function will allocate regardless of the current allocated size.
 *    * If DEBUG is defined, this function will print a debug message.
 */
StringT *
String_pre_allocated(char *str, ssize_t size) {
    char *string = malloc(size * (sizeof *string));
    StringT *self = malloc(sizeof *self);

    if (string == NULL) ERR("Unable to allocate memory for `StringT`");

    *self = (StringT){.string = str, .length = c_string_length(str), .allocated = size};

    return self;
}

/**
 * Compare a ``StringT`` object with a C string.
 * Returns ``true`` if the two strings are equal, ``false`` otherwise.
 *
 * .. note:: The ``char *`` must be NULL terminated.
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello");
 *    assert(String_eq(string, "Hello"));
 */
bool
String_eq(const StringT *self, const char *other) {
    for (ssize_t i = 0; i < self->length; ++i) {
        if (self->string[i] != other[i]) return false;
    }

    return true;
}

/**
 * Create a copy of existing ``StringT`` object.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *copy = String_copy(string);
 *    assert(String_eq(string, copy));
 */
StringT *
String_copy(const StringT *self) {
    return String_from(self->string);
}

/**
 * Push a character to the end of the ``StringT`` object.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    String_push(string, '!');
 *    assert(String_eq(string, "Hello, World!"));
 */
static void
String_push(StringT *self, char ch) {
    String_re_allocate(self, self->length + 1);

    self->string[self->length] = ch;
    self->length++;
}

/**
 * Deep free the ``StringT`` object.
 *
 * .. note::
 *   * This function will free the base string.
 */
void
String_free(StringT *self) {
    free(self->string);
    free(self);
}

/**
 * Get the character at the given index.
 * Negative indices are supported.
 *
 * .. note:: If index is out of range, this function will throw an error and exit.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    assert(String_index(string, 0) == 'H');
 *    assert(String_index(string, -1) == '!');
 */
char
String_index(const StringT *self, ssize_t index) {
    index = negative_index_to_positive(index, self->length);

    return self->string[index];
}

/**
 * Get the slice of the ``StringT`` object.
 *
 * .. note:: If index is out of range, this function will throw an error and exit.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *slice1 = String_slice(string, StringIndex(0, 5));
 *    StringT *slice2 = String_slice(string, StringIndex(7, -1));
 *    StringT *slice3 = String_slice(string, StringIndex(0, -1, 2));
 *
 *    assert(String_eq(slice1, "Hello"));
 *    assert(String_eq(slice2, "World!"));
 *    assert(String_eq(slice3, "Hlo ol!"));
 */
StringT *
String_slice(const StringT *self, StringIndexT index) {
    StringT *slice = String_new(0);

    index = StringIndex_normalize(index, self->length);
    int slice_length = StringIndex_len(index);

    while (slice_length--) {
        String_push(slice, self->string[index.start]);
        index.start += index.step;
    }

    return slice;
}

/**
 * Concatenate two strings and return the concatenated string.
 *
 * .. note:: This function will not modify the original strings.
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("Hello, ");
 *    StringT *string2 = String_from("World!");
 *    StringT *concatenated_string = String_concatenate(string1, string2);
 *
 *    assert(String_eq(concatenated_string, "Hello, World!"));
 */
StringT *
String_concatenate(const StringT *self, const StringT *other) {
    StringT *concatenated_string = String_copy(self);

    String_re_allocate(concatenated_string, self->length + other->length);

    for (ssize_t i = 0; i < other->length; ++i)
        concatenated_string->string[i + self->length] = other->string[i];

    concatenated_string->length += other->length;

    return concatenated_string;
}

/**
 * Concatenate two strings and store the result in the first string.
 *
 * .. note:: This function will modify the first string.
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("Hello, ");
 *    StringT *string2 = String_from("World!");
 *    String_concatenate_inplace(string1, string2);
 *
 *    assert(String_eq(string1, "Hello, World!"));
 */
void
String_concatenate_inplace(StringT *self, const StringT *other) {
    String_re_allocate(self, self->length + other->length);

    for (ssize_t i = 0; i < other->length; ++i)
        self->string[self->length++] = other->string[i];
}

/**
 * Repeat the string ``times`` times and return the new string.
 *
 * .. note:: This function will not modify the original string.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, ");
 *    StringT *repeated_string = String_repeat(string, 3);
 *
 *    assert(String_eq(repeated_string, "Hello, Hello, Hello, "));
 */
StringT *
String_repeat(const StringT *self, ssize_t times) {
    ssize_t new_length;
    StringT *new_string;

    if (times < 0) return String_new(0);

    new_length = self->length * times;
    new_string = String_pre_allocated(self->string, new_length);

    for (ssize_t i = 1; i < times; ++i) String_concatenate_inplace(new_string, self);
    new_string->string[new_length] = '\0';

    return new_string;
}


/**
 * Check if two ``StringT`` objects are equal.
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("Hello, ");
 *    StringT *string2 = String_from("World!");
 *    StringT *string3 = String_from("World!");
 *
 *    assert(!String_equals(string1, string3));
 *    assert(String_equals(string2, string3));
 */
bool
String_equals(const StringT *self, const StringT *other) {
    return String_eq(self, other->string);
}

/**
 * Internal function to construct the bad match table which holds the relative positions
 * of the letters with respect to their last occurrence in the pattern.
 * For string ``"ABCADABCAC"`` the bad match table will look like
 * ``{A: 1, C: 1, B: 3, D: 5}``.
 * These are currently stored in a ``ssize_t`` array of size ``U8_MAX``.
 *
 * .. todo:: Get rid of empty spaces in the arary.
 */
static void
_construct_bad_match_table(const StringT *self, ssize_t *match_table) {
    for (ssize_t i = 0; i < self->length; ++i) {
        match_table[(int)self->string[i]] = MAX_2(1, self->length - i - 1);
    }
}

/**
 * Internal function to compare the substring with the string from a given starting point
 * in reverse order.
 */
static bool
_reverse_string_compare_from_starting_point(const StringT *self, const StringT *sub,
                                            size_t start) {
    ssize_t j = sub->length - 1;
    for (ssize_t i = start; j >= 0 && self->string[i] == sub->string[j]; --i, --j)
        ;
    return j < 0;
}

/**
 * Check if the given substring is contained within the original string.
 * Time complexity is O(n*m) where n is the length of the string and m is the length of
 * the sub_string.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *sub_string1 = String_from("World");
 *    StringT *sub_string2 = String_from("World!");
 *
 *    assert(String_contains(string, sub_string1));
 *    assert(!String_contains(string, sub_string2));
 */
StringIndexT
String_contains(const StringT *self, const StringT *other) {
    return String_contains_in_range(self, other, StringIndex(self->length));
}

/**
 * Check if the given substring is contained within the original string in the given
 * range.
 *
 * .. note:: Implementation is based on the `Boyer Moore Horspool algorithm`_.
 * .. _Boyer Moore Horspool algorithm:: https://en.wikipedia.org/wiki/Boyer–Moore–Horspool_algorithm
 */
StringIndexT
String_contains_in_range(const StringT *self, const StringT *other, StringIndexT index) {
    StringIndexT not_found = StringIndex(0, 0, 1);
    ssize_t match_table[U8_MAX] = {0};
    ssize_t shift;

    _construct_bad_match_table(other, match_table);

    if (index.step != 1) ERR("String_contains_in_range: step must be 1");

    for (ssize_t i = index.start + other->length - 1; i < index.stop;) {
        shift = match_table[(int)self->string[i]];

        if (!shift)
            shift = other->length;
        else if (_reverse_string_compare_from_starting_point(self, other, i))
            return StringIndex(i - other->length + 1, i + 1);

        i += shift;
    }

    if (index.stop - index.start < other->length) return not_found;

    return not_found;
}

/**
 * Check if the string contains the given char and return the index of the first
 * occurrence.
 *
 * .. note::
 *    * This function will return the first occurrence of the character.
 *    * If the character is not found, it will return ``StringIndex(0, 0, 1)``.
 *
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringIndexT index1 = String_contains_char(string, 'W');
 *    StringIndexT index2 = String_contains_char(string, 'w');
 *
 *    assert(StringIndex_eq(index1, StringIndex(7, 8)));
 *    assert(StringIndex_eq(index2, StringIndex(0, 0, 1)));
 */
StringIndexT
String_contains_char(const StringT *self, const char character) {
    return String_contains_char_in_range(self, character, StringIndex(self->length));
}

/**
 * Check if the string contains the given char in the given range and return the index
 * of the first occurrence.
 *
 * .. note::
 *    * This function will return the first occurrence of the character.
 *    * If the character is not found, it will return ``StringIndex(0, 0, 1)``.
 *    * The step must be 1.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringIndexT index1 = String_contains_char_in_range(string, 'W', StringIndex(0, 5));
 *    StringIndexT index2 = String_contains_char_in_range(string, 'l', StringIndex(0, 5));
 *
 *    assert(StringIndex_eq(index1, StringIndex(0, 0, 1)));
 *    assert(StringIndex_eq(index2, StringIndex(2, 3)));
 */
StringIndexT
String_contains_char_in_range(const StringT *self, const char character,
                              StringIndexT index) {
    if (index.step != 1) ERR("String_contains_char_in_range: step must be 1");
    for (ssize_t i = index.start; i < index.stop; i += index.step)
        if (self->string[i] == character) return StringIndex(i, i + 1);
    return StringIndex(0, 0, 1);
}

/**
 * Check if the string contains any char from the char class and return the index of the
 * first occurrence.
 * Similar to regex: ``[...]``
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *char_class = String_from("Wd");
 *    StringIndexT index = String_contains_char_class(string, char_class);
 *
 *    assert(StringIndex_eq(index, StringIndex(7, 8)));
 */
StringIndexT
String_find_from_char_class(const StringT *self, const StringT *characters) {
    return String_find_from_char_class_in_range(self, characters,
                                                StringIndex(self->length));
}

/**
 * Check if the string contains any char from the char class in the given range and
 * return the index of the first occurrence.
 * Similar to regex: ``[...]``
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *char_class = String_from("Wd");
 *    StringIndexT index = String_contains_char_class_in_range(string, char_class,
 *                                                             StringIndex(0, 5));
 *
 *    assert(StringIndex_eq(index, StringIndex(0, 0, 1)));
 */
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

/**
 * Replace all occurrences of the substring with the replacement string.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *sub_string = String_from("l");
 *    StringT *replacement = String_from("L");
 *    StringT *new_string = String_replace(string, sub_string, replacement);
 *
 *    assert(String_eq(new_string, String_from("HeLLo, WorLd!")));
 */
StringT *
String_replace(const StringT *self, const StringT *sub_string,
               const StringT *replacement) {
    StringIteratorT *iterator = String_split(self, sub_string);
    return String_join(iterator, replacement);
}

/**
 * Split the string by the delimiter for a fixed ``limit`` and return a list of strings.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *delimiter = String_from("l");
 *    StringIteratorT *iterator = String_split_limit(string, delimiter, 1);
 *
 *    assert(StringIterator_len(iterator) == 2);
 *    assert(String_eq(StringIterator_get(iterator, 0), String_from("He")));
 *    assert(String_eq(StringIterator_get(iterator, 1), String_from("lo, World!")));
 */
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
        ERR("String_split_limit: limit must be greater than -1");
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

/**
 * Split the string by the delimiter and return a list of strings.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *delimiter = String_from("l");
 *    StringIteratorT *iterator = String_split(string, delimiter);
 *
 *    assert(StringIterator_len(iterator) == 3);
 *    assert(String_eq(StringIterator_get(iterator, 0), String_from("He")));
 *    assert(String_eq(StringIterator_get(iterator, 1), String_from("lo, Wor")));
 */
StringIteratorT *
String_split(const StringT *self, const StringT *delimiter) {
    return String_split_limit(self, delimiter, -1);
}

/**
 * Split the string based on newline chars.
 *
 * .. note:: Currently it only splits with the delimiter  ``'\n'``, other UTF-8
 *           newline chars not supported.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("foo\nbar\nspam\neggs");
 *    StringIteratorT lines = *String_split_lines(string);
 *
 *    assert(String_eq(StringIterator_next(lines), "foo");
 *    assert(String_eq(StringIterator_next(lines), "bar");
 *    assert(String_eq(StringIterator_next(lines), "spam");
 *    assert(String_eq(StringIterator_next(lines), "eggs");
 */
StringIteratorT *
String_split_lines(const StringT *self) {
    return String_split(self, String_from("\n"));
}

/**
 * Split the string based on whitespace chars.
 *
 * .. list-table:: Whitespace chars
 *    :width: 25 1
 *    :heading-rows: 1
 *
 *    * - Name
 *      - Representation
 *    * - Space
 *      - Newline
 *      - Carriage return
 *      - Form feed
 *      - Tab
 *    * - ' '
 *      - '\n'
 *      - '\r'
 *      - '\f'
 *      - '\t'
 *
 *
 *  .. code-block:: c
 *
 *     StringT *string = String_from("foo bar\nfoobar\tbar foo");
 *     StringT *strings = String_split_whitespace(string);
 *
 *     assert(String_eq(StringIterator_next(strings), "foo"));
 *     assert(String_eq(StringIterator_next(strings), "bar"));
 *     assert(String_eq(StringIterator_next(strings), "foobar"));
 *     assert(String_eq(StringIterator_next(strings), "bar"));
 *     assert(String_eq(StringIterator_next(strings), "foo"));
 */
StringIteratorT *
String_split_whitespace(const StringT *self) {
    return String_split_whitespace_limit(self, -1);
}

/**
 * Split the string based on newline chars for a fixed ``limit``.
 * See :func:`String_split_lines` for more info.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("foo\nbar\nspam\neggs");
 *    StringIteratorT lines = *String_split_lines_limit(string, 2);
 *
 *    assert(StringIterator_len(lines) == 2);
 *    assert(String_eq(StringIterator_next(lines), "foo");
 *    assert(String_eq(StringIterator_next(lines), "bar\nspam\neggs");
 */
StringIteratorT *
String_split_lines_limit(const StringT *self, ssize_t limit) {
    return String_split_limit(self, String_from("\n"), limit);
}

/**
 * Split the string based on whitespace chars for a fixed ``limit``.
 * See :func:`String_split_whitespace` for more info.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("foo bar\nfoobar\tbar foo");
 *    StringT *strings = String_split_whitespace_limit(string, 2);
 *
 *    assert(StringIterator_len(strings) == 2);
 *    assert(String_eq(StringIterator_next(strings), "foo"));
 *    assert(String_eq(StringIterator_next(strings), "bar\nfoobar\tbar foo");
 */
StringIteratorT *
String_split_whitespace_limit(const StringT *self, ssize_t limit) {
    StringIteratorT *iterator = StringIterator_new();
    ssize_t start = 0;

    // Special case for ``limit``
    // If limit is -1, then we iterate until the string is exhausted
    if (!limit) {
        StringIterator_append(iterator, self);
        return iterator;
    } else if (limit == -1) {
        limit = self->length;
    } else if (limit < -1) {
        ERR("String_split_limit: limit must be greater than -1");
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

/**
 * Split the a slice of the string based on a delimiter.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World!");
 *    StringT *delimiter = String_from(", ");
 *    StringIteratorT *strings = String_split_in_range(string, delimiter, StringIndex(0, 5));
 *
 *    assert(StringIterator_len(strings) == 1);
 *    assert(String_eq(StringIterator_next(strings), "Hello"));
 */
StringIteratorT *
String_split_in_range(const StringT *self, const StringT *delimiter, StringIndexT index) {
    return String_split(String_slice(self, index), delimiter);
}

/**
 * Split the string from right to left based on a delimiter.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("foo bar, spam, eggs");
 *    StringT *delimiter = String_from(", ");
 *    StringIteratorT *strings = String_right_split(string, delimiter);
 *
 *    assert(StringIterator_len(strings) == 2);
 *    assert(String_eq(StringIterator_next(strings), " eggs"));
 *    assert(String_eq(StringIterator_next(strings), " spam"));
 *    assert(String_eq(StringIterator_next(strings), "foo bar"));
 */
StringIteratorT *
String_right_split(const StringT *self, const StringT *delimiter) {
    return String_right_split_limit(self, delimiter, -1);
}

/**
 * Split the string from right to left based on a delimiter for a fixed ``limit``.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("foo bar, spam, eggs");
 *    StringT *delimiter = String_from(", ");
 *    StringIteratorT *strings = String_right_split_limit(string, delimiter, 1);
 *
 *    assert(StringIterator_len(strings) == 2);
 *    assert(String_eq(StringIterator_next(strings), " eggs"));
 *    assert(String_eq(StringIterator_next(strings), "foo bar, spam"));
 */
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
        ERR("String_split_limit: limit must be greater than -1");
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

/**
 * Join the strings present in a ``StringIteratorT`` separated by a delimiter.
 *
 * .. code-block:: c
 *
 *    StringT *delimiter = String_from(", ");
 *    StringIteratorT *strings = StringIterator_new();
 *    StringIterator_append(strings, String_from("foo"));
 *    StringIterator_append(strings, String_from("bar"));
 *    StringIterator_append(strings, String_from("spam"));
 *    StringT *string = String_join(strings, delimiter);
 *
 *    assert(String_eq(string, "foo, bar, spam"));
 */
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

/**
 * Check if a slice of the string starts with the provided prefix.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *prefix = String_from("Hello");
 *
 *    assert(String_starts_with_in_range(string, prefix, StringIndex(0, 5)));
 */
static bool
String_check_equals_in_range(const StringT *self, const StringT *other,
                             StringIndexT index) {
    if (index.stop > self->length) {
        ERR("String_check_equals_in_range: index out of range");
    }

    if (other->length > self->length) return false;

    for (ssize_t i = index.start; i < index.stop; i += index.step)
        if (self->string[i] != other->string[i - index.start]) return false;

    return true;
}

/**
 * Check if the string starts with the provided prefix.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *prefix = String_from("Hello");
 *
 *    assert(String_starts_with(string, prefix));
 */
bool
String_starts_with(const StringT *self, const StringT *prefix) {
    return String_check_equals_in_range(self, prefix, StringIndex(prefix->length));
}

/**
 * Check if a string ends with the provided suffix.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *suffix = String_from("World");
 *
 *    assert(String_ends_with(string, suffix));
 */
bool
String_ends_with(const StringT *self, const StringT *suffix) {
    return String_check_equals_in_range(
        self, suffix, StringIndex(self->length - suffix->length, self->length));
}

/**
 * Reverse the string and return a new string.
 *
 * .. note:: This function has a time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *reversed_string = String_reverse(string);
 *
 *    assert(String_eq(reversed_string, "dlroW ,olleH"));
 */
StringT *
String_reverse(const StringT *self) {
    return String_slice(self, StringIndex(self->length - 1, -1, -1));
}

/**
 * Convert the string to uppercase and return the uppercase string.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *upper_string = String_to_upper(string);
 *
 *    assert(String_eq(upper_string, "HELLO, WORLD"));
 */
StringT *
String_to_upper(const StringT *self) {
    StringT *new_string = String_copy(self);

    for (ssize_t i = 0; i < new_string->length; ++i)
        CHAR_TO_UPPERCASE(new_string->string[i]);


    return new_string;
}

/**
 * Convert the string to lowercase and return the lowercase string.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *lower_string = String_to_lower(string);
 *
 *    assert(String_eq(lower_string, "hello, world"));
 */
StringT *
String_to_lower(const StringT *self) {
    StringT *new_string = String_copy(self);

    for (ssize_t i = 0; i < new_string->length; ++i)
        CHAR_TO_LOWERCASE(new_string->string[i]);

    return new_string;
}

/**
 * Convert the string to title case and return the title case string.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("hello, world");
 *    StringT *title_string = String_to_title(string);
 *
 *    assert(String_eq(title_string, "Hello, World"));
 */
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

/**
 * Convert the first character of the string to uppercase and return the
 * capitalized string.
 *
 * .. note:: Has time complexity of O(n) (the string needs to be copied).
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("hello, world");
 *    StringT *capital_string = String_to_capital(string);
 *
 *    assert(String_eq(capital_string, "Hello, world"));
 */
StringT *
String_to_capital(const StringT *self) {
    StringT *new_string = String_copy(self);

    if (!new_string->length) return new_string;

    CHAR_TO_UPPERCASE(new_string->string[0]);

    return new_string;
}

/**
 * Swap the case of the string and return the new string.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello, World");
 *    StringT *swapped_string = String_swap_case(string);
 *
 *    assert(String_eq(swapped_string, "hELLO, wORLD"));
 */
StringT *
String_swap_case(const StringT *self) {
    StringT *new_string = String_copy(self);

    for (ssize_t i = 0; i < new_string->length; ++i)
        CHAR_SWAP_CASE(new_string->string[i]);

    return new_string;
}

/**
 * Check if the string is alphanumeric.
 * String is alphanumeric if all the characters in the string are either
 * alphabets or digits.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("Hello World");
 *    StringT *string2 = String_from("HelloWorld");
 *
 *    assert(!String_is_alphanumeric(string));
 *    assert(String_is_alphanumeric(string2));
 */
bool
String_is_alphanumeric(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_ALPHANUMERIC(self->string[i])) return false;

    return true;
}

/**
 * Check if the string is alphabetic.
 * String is alphabetic if all the characters in the string are alphabets.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("Hello World");
 *    StringT *string2 = String_from("HelloWorld");
 *
 *    assert(!String_is_alphabetic(string1));
 *    assert(String_is_alphabetic(string2));
 */
bool
String_is_alphabetic(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_ALPHABET(self->string[i])) return false;

    return true;
}

/**
 * Check if the string is uppercase.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("HELLO WORLD");
 *    StringT *string2 = String_from("HelloWorld");
 *
 *    assert(String_is_uppercase(string1));
 *    assert(!String_is_uppercase(string2));
 */
bool
String_is_uppercase(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_UPPERCASE(self->string[i])) return false;

    return true;
}

/**
 * Check if the string is lowercase.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("hello world;
 *    StringT *string2 = String_from("HelloWorld");
 *
 *    assert(String_is_lowercase(string1));
 *    assert(!String_is_lowercase(string2));
 */
bool
String_is_lowercase(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_LOWERCASE(self->string[i])) return false;

    return true;
}

/**
 * Check if the string is a valid integer.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("123");
 *    StringT *string2 = String_from("123.45");
 *
 *    assert(String_is_int(string1));
 *    assert(!String_is_int(string2));
 */
bool
String_is_int(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_DIGIT(self->string[i])) return false;

    return true;
}

/**
 * Check if the string is a valid real number.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from("123.45");
 *    StringT *string2 = String_from("123");
 *    StringT *string3 = String_from("foo bar");
 *
 *    assert(String_is_real(string1));
 *    assert(String_is_real(string2));
 *    assert(!String_is_real(string3));
 */
bool
String_is_real(const StringT *self) {
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

/**
 * Check if the string is whitespace.
 *
 * .. note:: Has time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string1 = String_from(" \t\n\f\r");
 *    StringT *string2 = String_from("Hello World");
 *
 *    assert(String_is_whitespace(string1));
 *    assert(!String_is_whitespace(string2));
 */
bool
String_is_whitespace(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_WHITESPACE(self->string[i])) return false;

    return true;
}

/**
 * Trim whitespace from both sides of the string and return the
 * trimmed string.
 *
 * .. note:: Has worst case time complexity of O(n).
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("  Hello, World ");
 *    StringT *trimmed = String_trim_whitespace(string));
 *
 *    assert(String_equals(trimmed, String_from("Hello, World")));
 */
StringT *
String_trim_whitespace(const StringT *self) {
    return String_trim_left(String_trim_right(self));
}

/**
 * Trim whitespace left of the string.
 * Has worst case time complexity of O(n)
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("  Hello, World ");
 *    StringT *trimmed = String_trim_left(string));
 *
 *    assert(String_equals(trimmed, String_from("Hello, World ")));
 */
StringT *
String_trim_left(const StringT *self) {
    for (ssize_t i = 0; i < self->length; ++i)
        if (!CHAR_IS_WHITESPACE(self->string[i]))
            return String_slice(self, StringIndex(i, self->length));
    return String_copy(self);
}

/**
 * Trim whitespace right of the string.
 * Has worst case time complexity of O(n)
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("  Hello, World ");
 *    StringT *trimmed = String_trim_right(string));
 *
 *    assert(String_equals(trimmed, String_from("  Hello, World")));
 */
StringT *
String_trim_right(const StringT *self) {
    for (ssize_t i = self->length - 1; i >= 0; --i)
        if (!CHAR_IS_WHITESPACE(self->string[i]))
            return String_slice(self, StringIndex(i + 1));
    return String_copy(self);
}

/**
 * Pad the string with the given left and right padding.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello");
 *    StringT *padded = String_pad(string, 2, 2);
 *
 *    assert(String_equals(padded, String_from("  Hello  ")));
 */
StringT *
String_pad(const StringT *self, ssize_t left_pad, ssize_t right_pad) {
    StringT *left_padded =
        String_concatenate(String_repeat(String_from(" "), left_pad), self);
    StringT *right_padded =
        String_concatenate(left_padded, String_repeat(String_from(" "), right_pad));

    return right_padded;
}

/**
 * Centre the string within the given width.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hello");
 *    StringT *centred = String_centre(string, 9);
 *
 *    assert(String_equals(centred, String_from("  Hello  ")));
 */
StringT *
String_centre(const StringT *self, ssize_t width) {
    ssize_t margin, left_pad;

    if (self->length >= width) return String_copy(self);

    margin = width - self->length;
    left_pad = margin / 2;

    return String_pad(self, left_pad, margin - left_pad);
}

/**
 * Justify the text towards the left within the given width.
 * Padding is done with space chars
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hi");
 *    StringT *left_justified = String_left_justify(string, 4);
 *
 *    assert(String_equals(left_justified, String_from("Hi  ")));
 */
StringT *
String_left_justify(const StringT *self, ssize_t width) {
    ssize_t length_to_fill = width - self->length;

    if (length_to_fill < 0) return String_copy(self);

    // Concatenate the string with `length_to_fill` number of whitespaces
    return String_concatenate(self, String_repeat(String_from(" "), length_to_fill));
}

/**
 * Justify the text towards the right within the given width.
 * Padding is done with space chars
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hi");
 *    StringT *right_justified = String_right_justify(string, 4);
 *
 *    assert(String_equals(right_justified, String_from("  Hi")));
 */
StringT *
String_right_justify(const StringT *self, ssize_t width) {
    ssize_t length_to_fill = width - self->length;

    if (length_to_fill < 0) return String_copy(self);

    // Concatenate `length_to_fill` number of whitespaces with the string.
    return String_concatenate(String_repeat(String_from(" "), length_to_fill), self);
}

/**
 * Return a new string with the characters of the string
 * repeated `n` times.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hi");
 *    StringT *repeated = String_repeat(string, 3);
 *
 *    assert(String_equals(repeated, String_from("HiHiHi")));
 */
StringIteratorT *
String_chunks(const StringT *self, ssize_t chunk_size) {
    StringIteratorT *iterator = StringIterator_new();

    for (ssize_t i = 0; i < self->length; i += chunk_size)
        StringIterator_append(iterator,
                              String_slice(self, StringIndex(i, i + chunk_size)));

    return iterator;
}

/**
 * Return a new string with the characters of the string
 * repeated `n` times.
 *
 * .. code-block:: c
 *
 *    StringT *string = String_from("Hi");
 *    StringT *repeated = String_repeat(string, 3);
 *
 *    assert(String_equals(repeated, String_from("HiHiHi")));
 */
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
