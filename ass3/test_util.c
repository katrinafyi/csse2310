#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int main(int argc, char** argv) {
    printf("%s\n", string_int(12300));
    printf("%s\n", string_int(123456789));

    char* tokens[3];
    int numTokens = tokenise(argv[1], ',', tokens, 3);
    printf("found %d tokens\n", numTokens);
    for (int i = 0; i < numTokens; i++) {
        if (i >= 3) {
            break;
        }
        printf("%d: |%s|\n", i, tokens[i]);
    }
}
