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

// see header
void conn_set_files(Connection* connection, FILE* readFile, FILE* writeFile) {
    connection->readFile = readFile;
    connection->writeFile = writeFile;
}
