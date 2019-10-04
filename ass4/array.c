#define _GNU_SOURCE 1

#include <stdlib.h>
#include <assert.h>

#include "array.h"



// see header
void array_init(Array* array) {
    // initialise rw lock
    assert(pthread_rwlock_init(&array->lock, NULL) == 0);

    // initialise items to null pointers (0)
    array->items = calloc(ARRAY_INITIAL_SIZE, sizeof(void*));
    assert(array->items != NULL);

    array->numItems = 0;
    array->numAllocated = ARRAY_INITIAL_SIZE;
    array->sorter = NULL;
    array->mapper = NULL;
}

// see header
void arraymap_init(Array* array, ArrayMapper mapper, ArraySorter sorter) {
    // initialise array
    array_init(array);
    // add map functions
    array->sorter = sorter;
    array->mapper = mapper;
}

// see header
void array_destroy(Array* array) {
    // don't do anything to items themselves.
    if (array->items != NULL) {
        free(array->items);
    }

    pthread_rwlock_destroy(&array->lock);
}

// see header
void array_foreach(Array* array, void (*func)(void*)) {
    for (int i = 0; i < array->numItems; i++) {
        func(array->items[i]);
    }
}

// see header
void array_free_items(Array* array) {
    array_foreach(array, free);
}

// see header
int array_add(Array* array, void* item) {
    // double array size.
    if (array->numItems == array->numAllocated) {
        int newAlloc = 2 * array->numAllocated;
        void** newItems = realloc(array->items, newAlloc * sizeof(void*));
        assert(newItems != NULL);

        array->items = newItems;
        array->numAllocated = newAlloc;
    }

    array->items[array->numItems] = item;
    array->numItems++;

    return array->numItems;
}

// see header
void* array_get_at(Array* array, int index) {
    assert(0 <= index && index < array->numItems);
    void* item = array->items[index];
    return item;
}

// see header
void* arraymap_get(Array* arrayMap, void* key) {
    for (int i = 0; i < arrayMap->numItems; i++) {
        void* item = arrayMap->items[i];
        void* valKey = arrayMap->mapper(item);
        if (arrayMap->sorter(valKey, key) == 0) {
            return item;
        }
    }
    return NULL;
}

// see header
void array_remove_at(Array* array, int index) {
    for (int i = index + 1; i < array->numItems; i++) {
        // shift items after this index down by one index.
        array->items[i - 1] = array->items[i];
    }
    array->numItems--;
}

// see header
void array_remove(Array* array, void* item) {
    for (int i = 0; i < array->numItems; i++) {
        void* thisItem = array->items[i];
        if (item == thisItem) {
            array_remove_at(array, i);
            return;
        }
    }
    assert(0); // item was not found in array.
}

/* qsort_r calls the sort comparer with pointers to the values being sorted
 * but our array sorter takes the items themselvers (which are void*'s), so
 * this wrapper dereferences the pointers.
 * Also converts the 3rd data argument to an Array*.
 *
 * Returns negative, 0 or positive if a < b, a = b or a > b respectively.
 */
int qsort_sorter(const void* a, const void* b, void* data) {
    Array* array = (Array*) data;
    return array->sorter(
            array->mapper(*(void**)a), array->mapper(*(void**)b));
}

// see header
void arraymap_sort(Array* array) {
    qsort_r(array->items, array->numItems, sizeof(void*),
            qsort_sorter, array);
}

