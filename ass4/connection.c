#include <stdlib.h>
#include <string.h>

#include "connection.h"
#include "util.h"


// see header
void conn_init(Connection* connection, int port, char* name) {
    connection->port = port;
    connection->name = strdup(name);
    connection->readFile = NULL;
    connection->writeFile = NULL;
}

// see header
void conn_set_socket(Connection* connection, FILE* readFile, FILE* writeFile) {
    connection->readFile = readFile;
    connection->writeFile = writeFile;
}

// see header
void conn_destroy(Connection* connection) {
    if (connection == NULL) {
        return;
    }
    TRY_FREE(connection->name);
    
    if (connection->readFile != NULL) {
        fclose(connection->readFile);
        connection->readFile = NULL;
    }
    if (connection->writeFile != NULL) {
        fclose(connection->writeFile);
        connection->writeFile = NULL;
    }
}

