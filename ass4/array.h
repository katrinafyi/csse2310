#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define ARRAY_INITIAL_SIZE 32

/* Returns the item in the array at the given index. Item is cast to a POINTER
 * to the given type.
 */
#define ARRAY_ITEM(type, array, index) ((type*) (array_get_at(array, index)))
// excessive parens are for simpatico

// locks the array for writing
#define ARRAY_WRLOCK(array) pthread_rwlock_wrlock(&array->lock)
// locks the array for reading
#define ARRAY_RDLOCK(array) pthread_rwlock_rdlock(&array->lock)
// unlocks the array
#define ARRAY_UNLOCK(array) pthread_rwlock_unlock(&array->lock)

/* pointer item stored within the array. */
typedef void* ArrayItem;
/* pointer to an arbitrary key value of some item in the array. */
typedef void* ArrayKey;
// this is ONLY for documentation purposes. using void* everywhere is
// acceptable.

/* Key mapper function. Takes items from the array and maps it to some key
 * value.
 */
typedef ArrayKey (*ArrayMapper)(ArrayItem);

/* Sort function for the array map. Takes two item keys and returns negative if
 * a < b, 0 if a == b and positive if a > b. Essentially, a - b if a and b
 * are numeric.
 * Used for sorting and comparison purposes. Item keys passed to this function
 * are values returned by mapper function.
 */
typedef int (*ArraySorter)(ArrayKey, ArrayKey);



/* Basic array structure which automatically grows. To handle arbitrary types,
 * this stores void*'s.
 *
 * Using mapper and sorter, this doubles as a horrible map/dictionary
 * implementation.
 *
 * This stores a lock which we can use but these functions DO NOT lock by
 * themself.
 */
typedef struct Array {
    ArrayItem* items; // items in array, actually just a list of void*'s
    int numItems; // number of items in array
    int numAllocated; // space allocated. should be >= numItems
    ArrayMapper mapper; // maps items to some key.
    ArraySorter sorter; // comparison function on the keys.
    pthread_rwlock_t lock; // rwlock for synchronising reads/writes.
} Array;

/* Initialises an array with the default initial size.
 */
void array_init(Array* array);

/* Initialises an array map with the given mapper and sorter functions,
 * as defined above.
 */
void arraymap_init(Array* array, ArrayMapper mapper, ArraySorter sorter);

/* Destroy the array, freeing memory used to hold its items.
 * Does not free any of the items.
 */
void array_destroy(Array* array);

/* Destroy the array as above and free each contained item.
 */
void array_destroy_and_free(Array* array);

/* Calls the given function once for every element in the array. The function
 * should take one item (as a void*) and return nothing.
 */
void array_foreach(Array* array, void (*func)(void*));

/* Frees all pointers stored in the array. Essentially the same as
 * array_foreach(array, free);
 */
void array_free_items(Array* array);

/* Append the given item to the end of the array.
 */
void array_add(Array* array, void* item);

/* Takes a copy of *item which has size size and stores it into a new MALLOC'd
 * pointer. Appends this new pointer to the array and returns it.
 */
void* array_add_copy(Array* array, void* item, size_t size);

/* Get the item at the given index, asserting the index is valid.
 */
void* array_get_at(Array* array, int index);

/* Returns the item which compares equal to the given key value, or null if
 * nothing matched.
 */
void* arraymap_get(Array* arrayMap, void* key);

/* Remove the given item from array, asserting item is in array.
 */
void array_remove(Array* array, void* item);

/* Remove the item at the given index, asserting index valid.
 */
void array_remove_at(Array* array, int index);

/* Sorts the array map in-place, in the total ordering defined by sortKey.
 */
void arraymap_sort(Array* array);

#endif
