#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "util.h"

// tests util functions.
int main(int argc, char** argv) {
    // test stringify int
    printf("%s\n", int_to_string(12300));
    printf("%s\n", int_to_string(123456789));

    char* tokens[3];
    assert(argc >= 2);
    // tests the new tokeniser
    int numTokens = tokenise(argv[1], ',', tokens, 3);
    printf("found %d tokens\n", numTokens);
    for (int i = 0; i < numTokens; i++) {
        if (i >= 3) {
            break;
        }
        printf("%d: |%s|\n", i, tokens[i]);
    }
}
