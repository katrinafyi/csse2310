#include "network.h"
#include "util.h"

// see header
bool new_socket(char* port, int* fdOut, struct addrinfo** aiOut) {
    // copied with few modifications from Joel's net4.c
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_flags = AI_PASSIVE; // server

    struct addrinfo* ai; // for address info returned by getaddrinfo
    int error = getaddrinfo("127.0.0.1", port, &hints, &ai);
    *aiOut = ai;

    if (error != 0) {
        DEBUG_PRINTF("error getaddrinfo: %s\n", gai_strerror(error));
        return false;
    }
    *fdOut = socket(AF_INET, SOCK_STREAM, 0); // 0 is default protocol
    return true;
}

// see header
bool start_active_socket(int* fdOut, char* port) {
    struct addrinfo* ai;
    int sock;
    // init new socket()
    if (!new_socket(port, &sock, &ai)) {
        return false;
    }
    if (connect(sock, ai->ai_addr, ai->ai_addrlen) != 0) {
        DEBUG_PERROR("connect()");
        freeaddrinfo(ai);
        return false;
    }
    freeaddrinfo(ai);
    *fdOut = sock;
    return true;
}

// see header
bool start_passive_socket(int* fdOut, int* portOut) {
    struct addrinfo* ai;
    int server;
    // init new socket()
    if (!new_socket(NULL, &server, &ai)) {
        return false;
    }
    if (bind(server, ai->ai_addr, ai->ai_addrlen) != 0) {
        freeaddrinfo(ai);
        DEBUG_PERROR("bind()");
        return false;
    }
    freeaddrinfo(ai);
    ai = NULL;

    // find actual address we're bound to
    struct sockaddr_in ad;
    socklen_t len = sizeof(struct sockaddr_in);
    memset(&ad, 0, len);
    if (getsockname(server, (struct sockaddr*)&ad, &len) != 0) {
        DEBUG_PERROR("getsockname()");
        return false;
    }
    DEBUG_PRINTF("bound to address %s port %d\n", inet_ntoa(ad.sin_addr),
            ntohs(ad.sin_port));

    if (listen(server, CONNECTION_QUEUE) != 0) {
        DEBUG_PERROR("listen()");
        return false;
    }

    *fdOut = server;
    *portOut = ntohs(ad.sin_port);
    return true;
}

