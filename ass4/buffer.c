#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "channel.h"

// line buffer for incoming data
#define LINE_BUFFER 512

/* Sleeps for the given amount of milliseconds. Will be inaccurate if an
 * interrupt is received. Will fail if msec >= 1000.
 */
void sleep_ms(unsigned int msec) {
    struct timespec ts = {0};
    ts.tv_nsec = msec * 1000000; // milliseconds to nanoseconds
    nanosleep(&ts, &ts);
}

/* Thread to read incoming data from stdin. Takes argument of channel to write
 * received data to. Return value unused, returns on EOF.
 */
void* reader_thread(void* channelArg) {
    Channel* channel = channelArg;

    char* line;
    while (1) {
        line = malloc(LINE_BUFFER * sizeof(char));
        fgets(line, LINE_BUFFER - 1, stdin);
        if (line[0] == '\0') {
            break; // break on EOF of input file
        }
        chan_post(channel, line);
    }
    return NULL;
}

/* Thread to write data from channel (given as void*) to stdout and stderr.
 * Does not return.
 */
void* writer_thread(void* channelArg) {
    Channel* channel = channelArg;

    while (1) {
        // enable cancellation only if there are no remaining items to process
        // WARNING: possible race condition.
        int items;
        sem_getvalue(&channel->numItems, &items);
        int cancelState = items == 0 ? PTHREAD_CANCEL_ENABLE
                : PTHREAD_CANCEL_DISABLE;
        pthread_setcancelstate(cancelState, NULL);

        char* line = chan_wait(channel);
        // disable cancellation while processing
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        fprintf(stdout, "%s", line);
        while (fflush(stdout) != 0) {
            sleep_ms(100);
        }
        fprintf(stderr, "%s", line);
        while (fflush(stderr) != 0) {
            sleep_ms(100);
        }
        free(line);
    }
}

/* Entry point of buffer. Sets up channels and threads. Terminads writer thread
 * when EOF is received on reader thread and there are no more lines to process
 */
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Channel channel = {0};
    chan_init(&channel);

    pthread_t readThread;
    pthread_t writeThread;

    pthread_create(&readThread, NULL, reader_thread, &channel);
    pthread_create(&writeThread, NULL, writer_thread, &channel);

    pthread_join(readThread, NULL);
    // close writer thread when reader is ended. writer will finish when there
    // are no items remaining.
    pthread_cancel(writeThread);
    pthread_join(writeThread, NULL);
    return 0;
}
