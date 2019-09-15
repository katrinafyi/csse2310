#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// used as an assert(false) which can't be disabled.
// for BIG mistakes.
#define INSTANT_SEGFAULT *((int*)0) = 42

// macros to print a message along with function and line number.
// unfortunately, these crash the style.sh
#define DEBUG_PRINT(str) fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, \
        str);
#define DEBUG_PRINTF(fmt, ...) fprintf(stderr, \
        "%s:%d "fmt, __func__, __LINE__, __VA_ARGS__);

/* Parses the str into a non-negative integer, with the following
 * requirements:
 *  - first character is + or a digit
 *  - contains decimal digits
 *  - no trailing whitespace or non-digit characters
 * Returns the integer on success or a negative number on failure.
 */
int parse_int(char* str);

/* Formats the given integer into a MALLOC'd string, returning the
 * string. Should always succeed.
 */
char* int_to_string(int number);

/* Helper method to read an arbitrary length line from a file. Ensures that
 *  - file is not NULL
 *  - line does not contain \0 characters
 *  - no IO errors occur
 * Returns false if any of the above conditions fail, true on success.
 *
 * MALLOCs enough space for the line. Stores the allocated pointer into
 * *output. Space is only allocated if the return value is true.
 */
bool safe_read_line(FILE* file, char** output);

/* Splits the input string by the split character, returning the number of
 * tokens in the string. Stores the start of each token into the given tokens
 * array.
 *
 * Returns the number of tokens actually in the line. However, only up to
 * numTokens tokens will be stored into tokens.
 *
 * Replaces split chars in the string with \0, so for i = 0, ..., numTokens-1,
 * tokens[i] points to the start of the i-th token which can be
 * treated as an individual \0 terminated string.
 */
int tokenise(char* line, char split, char** tokens, int numTokens);

// The style.sh chokes on our DEBUG_PRINT macros up top, so we replace them
// with these noop functions when building in 'release' mode.

/* Stubs to placate the style checker. */
void noop_print(char* str);

/* Stubs to placate the style checker. */
void noop_printf(char* fmt, ...);

#endif
