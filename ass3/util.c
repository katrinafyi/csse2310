#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <signal.h>

#define LINE_BUFFER 10

// see header
int parse_int(char* str) {
    // reject unless leading char is digit or +
    if (!(isdigit(*str) || *str == '+')) {
        DEBUG_PRINT("invalid first char");
        return -1;
    }
    char* end;
    errno = 0;
    int num = (int)strtol(str, &end, 10); // base 10
    // original string is empty or not complete match.
    // invalid number.
    if (errno != 0 || *end != '\0') {
        DEBUG_PRINT("check error");
        return -1;
    }
    return num;
}

// see header
char* int_to_string(int number) {
    int len = snprintf(NULL, 0, "%d", number);
    char* str = malloc((len + 1) * sizeof(char));
    snprintf(str, len + 1, "%d", number);
    return str;
}

// see header
bool safe_read_line(FILE* file, char** output) {
    if (file == NULL) {
        return false;
    }
    int allocated = LINE_BUFFER; // allocates space in LINE_BUFFER chunks
    *output = malloc(sizeof(char) * allocated);
    int position = 0;
    int next;
    while (1) {
        errno = 0;
        next = fgetc(file);
        if (errno != 0 || next == EOF || next == '\n') {
            (*output)[position] = '\0';
            break;
        } else {
            (*output)[position] = (char)next;
            position++;

            if (position >= allocated) {
                allocated *= 2; // for amortised constant time
                // printf("allocating to %d\n", allocated);
                *output = realloc(*output, sizeof(char) * allocated);
            }
        }
    }
    // free up space we don't need
    // *output = realloc(*output, sizeof(char) * (position + 1));
    // if line contains nulls, strlen will be < position.
    bool isValid = errno == 0 && strlen(*output) == position;
    // if we got EOF, this is only valid at the end of a line,
    // i.e. if position is non-zero
    isValid = isValid && (next != EOF || position > 0);
    if (!isValid) {
        // discard line if input is invalid.
        free(*output);
        *output = NULL;
    }
    return isValid;
}

// see header
int tokenise(char* line, char split, char** tokens, int maxTokens) {
    int len = strlen(line);
    int curNumTokens = 1;
    tokens[0] = line; // first token start at start of string.
    //DEBUG_PRINTF("tokenising: |%s|\n", line);
    for (int i = 0; i <= len; i++) {
        if (curNumTokens >= maxTokens) {
            break;
        }

        char c = line[i];
        if (c == split) {
            // replace split character with \0
            // each token can be viewed as its own null terminated string.
            line[i] = '\0';
            //DEBUG_PRINTF("token end at %d\n", i);

            // next token starts after this character.
            // if line ends with a split character,
            // last token will be an empty string.
            tokens[curNumTokens] = line + i + 1;
            curNumTokens++;
        }
    }
    return curNumTokens;
}

// see header
struct sigaction new_sigaction(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    return sa;
}

// see header
void ignore_sigpipe(void) {
    struct sigaction sa = new_sigaction();

    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);
}

// see header
void noop_print(char* str) {
    ;
}

// see header
void noop_printf(char* fmt, ...) {
    ;
}
