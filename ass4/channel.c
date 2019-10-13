#include <stdlib.h>

#include "channel.h"

// increments parameter by 1 and wraps it around the channel size
#define CHANNEL_INCREMENT(n) (((n) + 1) % CHANNEL_SIZE)

// code heavily based on week 11's practical code, with some modifications.
// because we don't really need a fixed-length circular queue by itself, 
// we just build it into the channel here.

// see header
void chan_init(Channel* channel) {
    channel->insertPos = 0;
    channel->readPos = 0;

    pthread_mutex_init(&channel->lock, NULL); // default attrs

    // last 2 params are shared and initial value.
    // not shared because within same process, and start at 0.
    sem_init(&channel->numItems, 0, 0);
    sem_init(&channel->numFree, 0, CHANNEL_SIZE);

    channel->initialised = true;
}

// see header
void chan_destroy(Channel* channel) {
    if (channel == NULL) {
        return;
    }
    if (channel->initialised) {
        pthread_mutex_destroy(&channel->lock);
        sem_destroy(&channel->numItems);
        sem_destroy(&channel->numFree);

        channel->initialised = false;
    }
}

// see header
void chan_foreach(Channel* channel, void (*consumer)(void*)) {
    for (int i = channel->readPos; i != channel->insertPos; 
            i = CHANNEL_INCREMENT(i)) {
        consumer(channel->items[i]);
    }
}

// see header
void chan_post(Channel* channel, void* item) {
    // careful with order of posting free/occupied semaphores
    sem_wait(&channel->numFree);
    pthread_mutex_lock(&channel->lock);

    channel->items[channel->insertPos] = item;
    channel->insertPos = CHANNEL_INCREMENT(channel->insertPos);

    pthread_mutex_unlock(&channel->lock);
    sem_post(&channel->numItems);
}

// see header
void* chan_wait(Channel* channel) {
    sem_wait(&channel->numItems);
    pthread_mutex_lock(&channel->lock);

    void* item = channel->items[channel->readPos];
    channel->readPos = CHANNEL_INCREMENT(channel->readPos);

    pthread_mutex_unlock(&channel->lock);
    sem_post(&channel->numFree);
    return item;
}
