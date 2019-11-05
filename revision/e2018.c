#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char* read_line(FILE* f) {
    int len = 0;
    char* line = malloc(sizeof(char));

    char c;
    while (1) {
        c = fgetc(f);
        if (c == EOF || c == '\n') {
            break;
        }
        line[len] = c;
        len++;
        line = realloc(line, sizeof(char) * (len+1));
    }
    line[len] = '\0';
    if (len == 0 && c == EOF)
        return NULL;
    return line;
}

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

void run_part(char** argv, FILE* a, FILE* b) {
    if (fork() == 0) { // in child
        dup2(fileno(a), STDIN_FILENO);
        dup2(fileno(b), STDOUT_FILENO);

        execvp(argv[0], argv);
    }
}

void run_cmd(char** cmds, char* a, char* b) {
    FILE* in = fopen(a, "r");
    FILE* out = NULL;

    int n = 0;
    while (cmds[n] != NULL)
        n++;

    for (int i = 0; i < n; i++) {
        int fd[2];
        if (i < n-1) {
            pipe(fd);
            out = fdopen(fd[1], "w");
        } else {
            out = fopen(b, "w");
        }
        char** argv = split_string(cmds[i]);
        run_part(argv, in, out);
        if (i < n-1) {
            in = fdopen(fd[0], "r");
        }
    }
}

int main(int argc, char** argv) {
    char* cmds[4] = {"ls -l", "cat", NULL};
    //run_cmd(cmds, "a", "b");
    while (1) {
        char* s = read_line(stdin);
        if (s == NULL) break;
        printf("|%s|\n", s);
    }
    return 0;
}
