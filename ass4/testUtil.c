#include <stdio.h>
#include <stdlib.h>

#include "util.h"

// test util functions for ass4
int main(void) {
    // testing asprintf works correctly.
    char* str = asprintf("str|%s| d|%d|\n", "abcdefghijk", 29979);
    printf("str{%s}\n", str);
    free(str);
}
