#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

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
    int num = (int)strtol(str, &end, 10);
    // original string is empty or not complete match.
    // invalid number.
    if (errno != 0 || *end != '\0') {
        DEBUG_PRINT("check error");
        return -1;
    }
    return num;
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
                allocated += LINE_BUFFER;
                // printf("allocating to %d\n", allocated);
                *output = realloc(*output, sizeof(char) * allocated);
            }
        }
    }
    // free up space we don't need
    *output = realloc(*output, sizeof(char) * (position + 1));
    // if line contains nulls, strlen will be < position.
    bool isValid = errno == 0 && strlen(*output) == position;
    if (!isValid) {
        // discard line if input is invalid.
        free(*output);
        *output = NULL;
    }
    return isValid;
}

// see header
int tokenise(char* line, int** indexes) { // TODO: specify fixed numTokens
    int len = strlen(line);
    int numTokens = 1;
    *indexes = malloc(sizeof(int) * numTokens);
    (*indexes)[0] = 0; // first token start at start of string.
    DEBUG_PRINTF("tokenising: |%s|\n", line);
    for (int i = 0; i <= len; i++) {
        char c = line[i];
        if (c == ' ') {
            // replace space with \0.
            // each token can be viewed as its own null terminated string.
            line[i] = '\0';
            // DEBUG_PRINTF("token at %d\n", i);

            // number of tokens should be small enough that realloc'ing
            // every token is fine.
            *indexes = realloc(*indexes, sizeof(int) * (numTokens + 1));
            // next token starts after this space.
            // if line ends with a space, last token will be an empty string.
            (*indexes)[numTokens] = i + 1;
            numTokens++;
        }
    }
    return numTokens;
}

// see header
void noop_print(char* str) {
    ;
}

// see header
void noop_printf(char* fmt, ...) {
    ;
}
