#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef DEBUG
#define DEBUG_FLAG 1
#else
#define DEBUG_FLAG 0
#endif

#define DEBUG_PRINT(str) if (DEBUG_FLAG) fprintf(stderr, "debug: "str"\n");
#define DEBUG_PRINTF(...) \
    if (DEBUG_FLAG) fprintf(stderr, "debugf: "__VA_ARGS__);

int parse_int(char* str);
bool safe_read_line(FILE* file, char** output);
int tokenise(char* line, int** indexes);

#endif

