#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>
#include <pthread.h>

#include "channel.h"

typedef struct Connection {
    int port;
    char* name; // malloc!
    pthread_t connectorThread; // thread which manages this connection
    pthread_t readThread; // thread which reads from this connection
    pthread_t writeThread; // thread which writes to this connection
    Channel* outgoing; // channel for outgoing messages, as Message*
} Connection;

/* Initialises a connection struct in the given location, with the given port
 * and name. A copy of name will be taken and stored in a MALLOC'd string.
 */
void conn_init(Connection* connection, int port, char* name);

/* Destroys a connection and frees its memory.
 */
void conn_destroy(Connection* connection);

/* Attaches the given thread IDs to this connection.
 */
void conn_set_threads(Connection* connection, pthread_t connectorThread,
        pthread_t readThread, pthread_t writeThread);

/* Cancels and joins the read and write threads of this connection.
 * Assumes readThread/writeThread are set correctly.
 */
void conn_cancel_threads(Connection* connection);
#endif
