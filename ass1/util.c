#include "util.h"

#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#define LINE_BUFFER 80

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

int safe_read_line(FILE* file, char** output) {
    if (file == NULL) {
        return 0;
    }
    int allocated = LINE_BUFFER;
    *output = malloc(sizeof(char)*allocated);
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
                *output = realloc(*output, sizeof(char)*allocated);
            }
        }
    }
    *output = realloc(*output, sizeof(char)*(position+1));
    // flags invalid line if line contains nulls
    // or an error occured.
    return errno == 0 && strlen(*output) == position;
}

