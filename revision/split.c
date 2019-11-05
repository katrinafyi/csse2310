#include <stdio.h>
#include <stdlib.h>

char** split_string(char* line) {
    char** split = malloc(sizeof(char*));
    char* left = line;
    int numSplits = 0;
    // loop over substrings inside line
    while (1) {
        // increment n until line[n] is blank or null
        int n = 0;
        while (left[n] != ' ' && left[n] != 0) {
            n++;
        }
        // if there was a non-empty string, store it into split
        if (n > 0) {
            split[numSplits] = malloc(sizeof(char)*(n+1));
            for (int i = 0; i < n; i++) {
                split[numSplits][i] = left[i];
            }
            split[numSplits][n] = '\0';

            numSplits++;
            split = realloc(split, sizeof(char*)*(numSplits+1));
        }
        // if we got a null, break
        if (left[n] == 0) break;
        // line[n] is a space, move left past this space
        left = left + n + 1;
    }
    // terminate array with NULL
    split[numSplits] = NULL;
    return split;
}

int main(int argc, char** argv) {
    char* s = "  1 2 3 4   asd   fds   f ";
    char** x = split_string(s);
    for (int i = 0; x[i] != NULL; i++) {
        printf("%d: |%s|\n", i, x[i]);
    }
    return 0;
}
