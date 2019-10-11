#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#define CHANNEL_SIZE 64

/* A synchronised channel for communication between an arbitrary number of
 * writers and an arbitrary number of readers.
 */
typedef struct Channel {
    void* items[CHANNEL_SIZE]; // array of items currently in channel
    int insertPos;
    int readPos;

    bool initialised; // flag indicating if semaphores are initialised
    pthread_mutex_t writeLock; // write lock for items
    sem_t numItems; // semaphore for items in queued channel
    sem_t numFree; // semaphore for amount of free space
} Channel;

/* Initialises a channel with a fixed capacity, initially empty.
 * Initialises synchronisation primitives.
 */
void chan_init(Channel* channel);

/* Destroys the channel struct itself, freeing memory. Does not implicitly
 * destroy contained items.
 */
void chan_destroy(Channel* channel);

/* Applies the given consumer function to each item in the channel.
 * Function should not affect number of items in the channel.
 * Parameter is called consumer because that's what Java calls a function of 
 * one argument returning nothing.
 */
void chan_foreach(Channel* channel, void (*consumer)(void*));

/* Posts the given item to the channel. Blocks until space is available in the
 * channel. The channel should be considered the 'owner' of the pointer, and is
 * now reponsible for freeing it. Argument SHOULD be a malloc'd pointer.
 */
void chan_post(Channel* channel, void* item);

/* Waits for an item in the given channel. Blocks until an item is available.
 * The caller is responsible for the returned pointer.
 */
void* chan_wait(Channel* channel);

#endif
