#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>
#include <pthread.h>

typedef struct Connection {
    int port;
    pthread_t readerThread;
    pthread_t writerThread;
    char* name; // malloc!
    Channel* outgoing; // channel for outgoing messages, as Message*
} Connection;

/* Initialises a connection struct in the given location, with the given port
 * and name. A copy of name will be taken and stored in a MALLOC'd string.
 */
void conn_init(Connection* connection, int port, char* name);

/* Destroys a connection and frees its memory.
 */
void conn_destroy(Connection* connection);


#endif
