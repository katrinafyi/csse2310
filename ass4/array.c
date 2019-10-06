#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "array.h"
#include "util.h"


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
void array_destroy_and_free(Array* array) {
    if (array == NULL) {
        return;
    }
    array_free_items(array);
    array_destroy(array);
}

// see header
void array_destroy(Array* array) {
    if (array == NULL) {
        return;
    }

    if (array->items != NULL) {
        // use array->items to indicate whether the lock is initialised.
        pthread_rwlock_destroy(&array->lock);
    }
    // don't automatically free() individual items
    TRY_FREE(array->items);
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
void array_add(Array* array, void* item) {
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
}

// see header
void* array_add_copy(Array* array, void* item, size_t size) {
    void* new = malloc(size);
    assert(new != NULL);
    // copy *item into *new, assuming item is of size given.
    memcpy(new, item, size);
    // add to array
    array_add(array, new);
    return new;
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

// see header
void arraymap_sort(Array* array) {
    // *grumble* C/POSIX compliance making me write a sorting algorithm.
    // we need to use mapper and sorter from the array struct to sort the
    // array correctly.

    // this is insertion sort, efficient for almost sorted arrays but O(n^2)
    // in worst-case.

    // invariant: A[:i] is sorted (using python slice notation) so we start
    // at i = 1 because a 1-element list is trivially sorted.
    for (int i = 1; i < array->numItems; i++) {
        // starting at j=i, we compare A[j] with A[j-1] and swap them if
        // A[j] should sort before A[j-1]. stop after j = 1 because j-1=0.
        // invariant: A[j:i] is sorted. when j=0 and we quit this loop, j=0
        // so this enforces the outer invariant.
        for (int j = i; j > 0; j--) {
            void* thisItem = array->items[j];
            void* thisKey = array->mapper(thisItem);

            void* othItem = array->items[j - 1];
            void* othKey = array->mapper(othItem);

            // currently, order is [..., othItem, thisItem, ...]
            if (array->sorter(othKey, thisKey) > 0) {
                // if othKey > thisKey, othItem is out of order. swap
                array->items[j] = othItem;
                array->items[j - 1] = thisItem;
            } else {
                // if othKey <= thisKey, we know thisKey >= all prior keys
                // as well. this gives O(n) on sorted arrays.
                break;
            }
        }
    }
}
