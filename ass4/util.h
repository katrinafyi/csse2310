#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/syscall.h>

// if ptr is non-null, frees it and sets it to null. otherwise, do nothing.
// useful in _destroy functions.
#define TRY_FREE(ptr) (ptr != NULL ? (free(ptr), ptr = NULL) : NULL)

// used as an assert(false) which can't be disabled.
// for BIG mistakes.
#define INSTANT_SEGFAULT() *((char**)0) = "manual segfault"

// terminal escape codes
#define TERM_GREY "\x1b[38;5;8m"
#define TERM_RED "\x1b[38;5;9m"
#define TERM_RESET "\x1b[0m"
#define TERM_REVERSE "\x1b[7m"

#define GET_TID() ((int)syscall(__NR_gettid))

#ifdef DEBUG

// macros to print a message along with function and line number. these are
// disabled by the sed_noop job in make.
// ass3: now with PID and colours
// ass4: fixed these crashing style.sh
#define DEBUG_PRINT(str) DEBUG_PRINTF(str"%c", '\n')
#define DEBUG_PRINTF(fmt, ...) fprintf(stderr,\
        "\x1b[38;5;%dm(%d %d) " TERM_GREY "%s:%d" TERM_RESET " " fmt,\
         GET_TID() % 7 + 9, getpid(), GET_TID() - getpid(),\
        __func__, __LINE__, __VA_ARGS__)
// formats in the style of:
// (pid) main:53 example message
//
// (pid) colour depends on the pid
// shows function and line number of location, with message.

#else

#define DEBUG_PRINT(str) NULL
#define DEBUG_PRINTF(fmt, ...) NULL

#endif

/* Parses the str into a non-negative integer, with the following
 * requirements:
 *  - first character is + or a digit
 *  - contains only decimal digits
 *  - no trailing whitespace or non-digit characters
 * Returns the integer on success or a negative number on failure.
 */
int parse_int(char* str);

/* Formats the given integer into a MALLOC'd string, returning the
 * string. Should always succeed.
 */
char* int_to_string(int number);

/* Formats the given format string and arguments into a new MALLOC'd string,
 * returning the string.
 */
char* asprintf(char* fmt, ...);

/* Helper method to read an arbitrary length line from a file. Ensures that
 *  - file is not NULL
 *  - line does not contain \0 characters
 *  - no IO errors occur
 *  - first read is not EOF
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
 *
 * Only splits into numTokens tokens. If the actual number of tokens is
 * greater than numTokens, the last token in tokens will contain the rest of
 * the string, unsplit.
 */
int tokenise(char* line, char split, char** tokens, int numTokens);

/* Intialises a new empty sigaction struct and returns it.
 */
struct sigaction new_sigaction(void);

/* Ignores the SIGPIPE signal via sigaction configuration.
 */
void ignore_sigpipe(void);

// The style.sh chokes on our DEBUG_PRINT macros up top, so we replace them
// with these noop functions when building in 'release' mode.
// ass4: not anymore but we keep these just in case.

/* Stubs to placate the style checker. */
void noop_print(char* str);

/* Stubs to placate the style checker. */
void noop_printf(char* fmt, ...);

#endif

