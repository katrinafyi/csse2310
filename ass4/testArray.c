#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

// identity function, returning exactly its argument
void* identity(void* item) {
    return item;
}

// prints a string
void print_item(void* item) {
    printf("%s,", (char*)item);
}

// program which sorts its arguments
int main(int argc, char** argv) {
    Array arrData = {0};
    Array* arr = &arrData;

    // initialise array map, sorted lexicographically.
    arraymap_init(arr, identity, (ArraySorter) strcmp);

    for (int i = 0; i < argc; i++) {
        array_add(arr, argv[i]);
    }
    printf("numItems: %d\n", arr->numItems);
    arraymap_sort(arr);
    array_foreach(arr, print_item);
    putchar('\n');

    void* item = arraymap_get(arr, "1");
    if (item == NULL) {
        puts("val not found");
    } else {
        printf("val found at %p: %s\n", item, (char*) item);
    }

    array_destroy(arr);
}
