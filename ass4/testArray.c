#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
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
    arraymap_init(arr, identity, strcmp_sorter);

    for (int i = 0; i < argc; i++) {
        array_add_copy(arr, argv[i], (strlen(argv[i]) + 1) * sizeof(char));
    }
    ARRAY_WRLOCK(arr); // test removal by removing argv[0]
    void* first = array_get_at(arr, 0);
    array_remove(arr, first);
    free(first);
    ARRAY_UNLOCK(arr);

    printf("numItems: %d\n", arr->numItems);
    arraymap_sort(arr);
    array_foreach(arr, print_item);

    putchar('\n');
    printf("item 0: %s\n", ARRAY_ITEM(char, arr, 0));
    DEBUG_PRINTF("%s\n", "hi simpatico!");

    void* item = arraymap_get(arr, "1");
    if (item == NULL) {
        puts("1 val not found");
    } else {
        printf("1 val found at %p: %s\n", item, (char*) item);
    }
    array_free_items(arr);
    array_destroy(arr);
    array_destroy(arr); // testing idempotence
}
