#include <stdlib.h>
#include <string.h>

#include "connection.h"
#include "arrayHelpers.h"
#include "messages.h"
#include "util.h"


// see header
void conn_init(Connection* connection, int port, char* name) {
    connection->port = port;
    connection->name = strdup(name);
    
    connection->outgoing = calloc(1, sizeof(Channel));
    chan_init(connection->outgoing);
}

// see header
void conn_destroy(Connection* connection) {
    if (connection == NULL) {
        return;
    }
    TRY_FREE(connection->name);

    if (connection->outgoing != NULL) {
        chan_foreach(connection->outgoing, ah_msg_destroy);
        chan_foreach(connection->outgoing, free);
        chan_destroy(connection->outgoing);
    }
    TRY_FREE(connection->outgoing);
}

// see header
void conn_set_threads(Connection* connection, pthread_t connectorThread,
        pthread_t readThread, pthread_t writeThread) {
    connection->connectorThread = connectorThread;
    connection->readThread = readThread;
    connection->writeThread = writeThread;
}

// see header
void conn_cancel_threads(Connection* connection) {
    pthread_cancel(connection->connectorThread);
    pthread_join(connection->connectorThread, NULL);

    pthread_cancel(connection->readThread);
    pthread_join(connection->readThread, NULL);

    pthread_cancel(connection->writeThread);
    pthread_join(connection->writeThread, NULL);
}
