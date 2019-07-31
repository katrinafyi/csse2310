#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG
#define DEBUG_FLAG 1
#else
#define DEBUG_FLAG 0
#endif

#define DEBUG_PRINT(str) if (DEBUG_FLAG) printf("debug: "str"\n");

int parse_int(char* str);
int safe_read_line(FILE* file, char** output);

#endif

