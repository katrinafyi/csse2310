#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Arg {
    char* big;
    char* srch;
    int tcount;
    int t;
    int out;
};

void* thread(void* param) {
    struct Arg* arg = param;
    int testLen = strlen(arg->big) / arg->tcount;
    int subLen = strlen(arg->srch);
    char* start = arg->big + testLen*arg->t;
    int bound = testLen;
    if (arg->t == arg->tcount-1) {
        bound = strlen(arg->big) - testLen*arg->t;
    }
    for (int i = 0; i < bound; i++) {
        printf("t%d-%d: %c\n", arg->t, bound, start[i]);
        if (strncmp(start+i, arg->srch, subLen) == 0) {
            arg->out++;
        }
    }
    return NULL;
}

int countsubstr(char* big, char* srch, int tcount) {
    struct Arg* args = calloc(tcount, sizeof(struct Arg));
    pthread_t* threads = calloc(tcount, sizeof(pthread_t));

    for (int t = 0; t < tcount; t++) {
        args[t].big = big;
        args[t].srch = srch;
        args[t].tcount = tcount;
        args[t].t = t;
        args[t].out = 0;
        pthread_create(threads+t, NULL, thread, args+t);
    }

    int out = 0;
    for (int t = 0; t < tcount; t++) {
        pthread_join(threads[t], NULL);
        out += args[t].out;
    }
    return out;
}

int main(int argc, char** argv) {
    printf("occurrences: %d\n", countsubstr(argv[1], argv[2], atoi(argv[3])));
    return 0;
}
