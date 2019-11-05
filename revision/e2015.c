#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char* read_line(FILE* f) {
    int len = 0;
    char* line = malloc(sizeof(char));
    int c;
    while (1) {
        c = fgetc(f);
        if (c == EOF || c == '\n') break;
        line[len++] = c;
        line = realloc(line, sizeof(char) * (1+len));
    }
    line[len] = '\0';
    if (len == 0 && c == EOF) return NULL;
    return line;
}

int compare(const void* a, const void* b) {
    return strcmp(*(char**)a, *(char**)b);
}

char** load_and_sort(FILE* f) {
    char** array = malloc(sizeof(char*));
    int len = 0;
    while (1) {
        char* line = read_line(f);
        if (line == NULL) break;
        int lineLen = strlen(line);
        if (lineLen > 0 && line[lineLen-1] == '\n') {
            line[lineLen-1] = '\0';
        }
        array[len++] = line;
        array = realloc(array, sizeof(char*) * (len+1));
    }
    array[len] = (char*)NULL;
    qsort(array, len, sizeof(char*), compare);
    return array;
}

struct ThreadArg {
    pthread_t tid;
    int n;
    char* filename;
};

void* sort_thread(void* arg) {
    struct ThreadArg* s = arg;
    FILE* f = fopen(s->filename, "r");
    char** array = load_and_sort(f);
    for (int i = 0; array[i] != NULL; i++) {
        if (i >= s->n) break;
        printf("%d: %s\n", i, array[i]);
    }
    return NULL;
}

int main(int argc, char** argv) {
    int n = atoi(argv[1]);
    int numThreads = argc - 2;
    struct ThreadArg* data = calloc(numThreads, sizeof(struct ThreadArg));
    for (int i = 0; i < numThreads; i++) {
        data[i].n = n;
        data[i].filename = argv[i+2];
        pthread_create(&data[i].tid, NULL, sort_thread, data + i);
    }
    for (int i = 0; i < numThreads; i++) {
        pthread_join(data[i].tid, NULL);
    }
    return 0;
}


