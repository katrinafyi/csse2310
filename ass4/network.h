#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// server listen queue length
#define CONNECTION_QUEUE 32

/* Creates a new TCP socket. Also retrives address info for localhost on the
 * given port. Stores a pointer to this addrinfo into aiOut and stores the
 * socket fd into fdOut.
 * Returns true on success, false otherwise.
 */
bool new_socket(char* port, int* fdOut, struct addrinfo** aiOut);

/* Starts a new active socket which attempts to connect to the given port (as
 * a string). If successful, returns true and stores socket's fd into fdOut.
 * Returns false on fail.
 */
bool start_active_socket(int* fdOut, char* port);

/* Starts a new passive socket which will listen on an ephemeral port.
 * If successful, returns true and stores socket's fd into fdOut and stores
 * port number (as int) into portOut.
 * Returns false on fail.
 */
bool start_passive_socket(int* fdOut, int* portOut);

#endif
