#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define INSTANT_SEGFAULT *((int*)0) = 42

#ifdef DEBUG
// macros to print a message along with function and line number.
// unfortunately, these crash the style.sh
#define DEBUG_PRINT(str) fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, \
        str); // style_deleteme
#define DEBUG_PRINTF(fmt, ...) fprintf(stderr, \
        "%s:%d "fmt, __func__, __LINE__, __VA_ARGS__); // style_deleteme

#else
// enforces semicolon after these macros
#define DEBUG_PRINT(str) do {} while (0)
#define DEBUG_PRINTF(fmt, ...) do {} while (0)
#endif

/* Parses the str into a non-negative integer, with the following 
 * requirements:
 *  - first character is + or a digit
 *  - contains decimal digits
 *  - no trailing whitespace or non-digit characters
 * Returns the integer on success or a negative number on failure.
 */
int parse_int(char* str);

/* Helper method to read an arbitrary length line from a file. Ensures that
 *  - file is not NULL
 *  - line does not contain \0 characters
 *  - no IO errors occur
 * Returns false if any of the above conditions fail, true on success.
 *
 * MALLOCs enough space for the line. Stores the allocated pointer into
 * *output.
 */
bool safe_read_line(FILE* file, char** output);

/* Splits the input line by space characters, returning the number of
 * space-separated tokens in the line. Stores a MALLOC'd array of the starting
 * indexes of each token into *indexes.
 *
 * Replaces spaces in the string with \0, so for i = 0, ..., numTokens-1,
 * line + (*indexes)[i] points to the start of the i-th token which can be
 * treated as an individual \0 terminated string.
 */
int tokenise(char* line, int** indexes);

#endif

