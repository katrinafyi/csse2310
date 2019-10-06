#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>

typedef struct Connection {
    int port;
    char* name; // malloc!
    FILE* readFile;
    FILE* writeFile;
} Connection;

/* Initialises a connection struct in the given location, with the given port
 * and name. A copy of name will be taken and stored in a MALLOC'd string.
 */
void conn_init(Connection* connection, int port, char* name);

/* Attaches the given read/write FILE*'s to this connection.
 */
void conn_set_socket(Connection* connection, FILE* readFile, FILE* writeFile);

/* Destroys a connection and frees its memory.
 */
void conn_destroy(Connection* connection);


#endif
