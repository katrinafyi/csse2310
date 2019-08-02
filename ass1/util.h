#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef DEBUG
#define DEBUG_PRINT(str) fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, \
        str);
#define DEBUG_PRINTF(fmt, ...) fprintf(stderr, \
        "%s:%d "fmt, __func__, __LINE__, __VA_ARGS__);

#else
#define DEBUG_PRINT(str) do {} while (0)
#define DEBUG_PRINTF(fmt, ...) do {} while (0)
#endif


int parse_int(char* str);
bool safe_read_line(FILE* file, char** output);
int tokenise(char* line, int** indexes);

#endif

