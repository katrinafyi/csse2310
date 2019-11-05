#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Arg {
    pthread_t tid;
    char* start;
    char* srch;
    int testLen;
    int result;
} Arg;

void* substr_thread(void* a) {
    Arg* arg = a;
    int len = strlen(arg->srch);
    arg->result = 0;
    for (int i = 0; i < arg->testLen; i++) {
        if (strncmp(arg->start + i, arg->srch, len) == 0) {
            arg->result++;
        }
    }
    return NULL; // result in struct
}

int countsubstr(const char* big, const char* srch, int tcount) {
    Arg* args = calloc(tcount, sizeof(Arg));

    int bigLen = strlen(big);
    int perThread = bigLen / tcount;

    char* start = big;
    for (int i = 0; i < tcount; i++) {
        args[i].start = start;
        args[i].srch = srch;
        // last thread needs to consider all remaining characters
        args[i].testLen = (i+1 < tcount) ? perThread : strlen(start);
        start += args[i].testLen; // move testLen characters to the right
        pthread_create(&args[i].tid, NULL, substr_thread, args + i);
    }

    int count = 0;
    for (int i = 0; i < tcount; i++) {
        pthread_join(args[i].tid, NULL);
        count += args[i].result;
    }
    free(args);
    return count;
}

int main(int argc, char** argv) {
    printf("count: %d\n", countsubstr(argv[1], argv[2], atoi(argv[3])));
    return 0;
}
