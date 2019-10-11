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

