#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>


#include "exitCodes.h"
#include "depotState.h"
#include "messages.h"
#include "util.h"

#define BANNED_NAME_CHARS " \n\r:"
#define CONNECTION_QUEUE 32

typedef struct ThreadData {
    int fd;
    DepotState* depotState;
} ThreadData;


void* depot_thread(void* threadDataArg) {
    ThreadData* threadData = threadDataArg;
    DepotState* depotState = threadData->depotState;
    int fdRead = threadData->fd;
    int fdWrite = dup(fdRead);

    free(threadDataArg); // we wont be needing this anymore
    DEBUG_PRINT("depot thread started");
    
    FILE* readFile = fdopen(fdRead, "r");
    FILE* writeFile = fdopen(fdWrite, "w");
    char* line;
    while (1) {
        fprintf(writeFile, "IM:100:depotname\n");
        fflush(writeFile);
        if (!safe_read_line(readFile, &line)) {
            break;
        }
        printf("received on %d: %s\n", fdRead, line);
        free(line);
    }
    fclose(readFile);
    fclose(writeFile);
    DEBUG_PRINT("thread ended");
    return NULL;
}

void start_depot_thread(DepotState* depotState, int fd) {
    ThreadData* arg = malloc(sizeof(ThreadData));
    arg->fd = fd;
    arg->depotState = depotState;

    pthread_t thread;
    pthread_create(&thread, NULL, depot_thread, arg);
}

bool new_socket(char* port, int* fdOut, struct addrinfo** aiOut) {
    // copied with few modifications from Joel's net4.c
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // tcp
    hints.ai_flags = AI_PASSIVE; // server

    struct addrinfo* ai; // for address info returned by getaddrinfo
    int error = getaddrinfo("localhost", port, &hints, &ai);
    if (error != 0) {
        DEBUG_PRINTF("getaddrinfo: %s\n", gai_strerror(error));
        return false;
    }

    *fdOut = socket(AF_INET, SOCK_STREAM, 0); // 0 is default protocol
    *aiOut = ai;
    return true;
}


bool start_active_socket(int* fdOut, char* port) {
    struct addrinfo* ai;
    int sock;
    // init new socket()
    if (!new_socket(port, &sock, &ai)) {
        return false;
    }
    if (connect(sock, ai->ai_addr, sizeof(struct sockaddr)) != 0) {
        DEBUG_PRINT("connect failed");
        return false;
    }
    *fdOut = sock;
    return true;
}


bool start_passive_socket(int* fdOut, int* portOut) {
    struct addrinfo* ai;
    int server;
    // init new socket()
    if (!new_socket(NULL, &server, &ai)) {
        return false;
    }
    if (bind(server, ai->ai_addr, sizeof(struct sockaddr)) != 0) {
        DEBUG_PRINTF("bind failed: %s\n", strerror(errno));
        return false;
    }
    freeaddrinfo(ai);

    // find actual address we're bound to
    struct sockaddr_in ad;
    socklen_t len = sizeof(struct sockaddr_in);
    memset(&ad, 0, len);
    if (getsockname(server, (struct sockaddr*)&ad, &len) != 0) {
        DEBUG_PRINT("error getsockname");
        return false;
    }
    DEBUG_PRINTF("bound to address %s port %d\n", inet_ntoa(ad.sin_addr),
            ntohs(ad.sin_port));

    if (listen(server, CONNECTION_QUEUE) != 0) {
        DEBUG_PRINTF("listen failed: %s\n", strerror(errno));
        return false;
    }

    *fdOut = server;
    *portOut = ntohs(ad.sin_port);
    return true;
}

sigset_t* blocked_sigset(void) {
    static sigset_t ss;
    sigemptyset(&ss);
    sigaddset(&ss, SIGHUP);
    return &ss;
}

void* sighup_thread(void* depotStateArg) {
    DepotState* depotState = depotStateArg;
    DEBUG_PRINT("sighup listener started");
    sigset_t* blocked = blocked_sigset();

    while (1) {
        int sig;
        sigwait(blocked, &sig);
        DEBUG_PRINTF("signal: %d %s\n", sig, strsignal(sig));
    }
}

DepotExitCode exec_server(DepotState* depotState) {
    DEBUG_PRINT("starting server listener");

    // block SIGHUP from all threads. we will pick it up via sigwait
    sigset_t* blocked = blocked_sigset();
    pthread_sigmask(SIG_BLOCK, blocked, NULL);

    int server;
    int port;
    // start server socket in listen mode
    if (!start_passive_socket(&server, &port)) {
        return false;
    }
    depotState->port = port;

    // start sighup thread
    pthread_t signalThread; // unused
    pthread_create(&signalThread, NULL, sighup_thread, depotState);

    int test;
    start_active_socket(&test, depotState->name);
    start_depot_thread(depotState, test);

    while (1) {
        // listen for connections
        int fd = accept(server, 0, 0);
        DEBUG_PRINT("accepted connection");
        start_depot_thread(depotState, fd);
    }

    return D_NORMAL;
}

/* Returns whether the given string is a valid depot or material name,
 * according to rules in spec.
 */
bool is_name_valid(char* name) {
    int len = strlen(name);
    char* banned = BANNED_NAME_CHARS;
    for (int i = 0; i < len; i++) {
        // check if any char in name appears in banned list.
        if (strchr(banned, name[i]) != NULL) {
            DEBUG_PRINTF("name invalid: |%s|\n", name);
            return false;
        }
    }
    return len > 0; // ensure name non-empty
}

/* Argument checks and initialises depot state. Bootstraps server listener. */
DepotExitCode exec_main(int argc, char** argv, DepotState* depotState) {
    if (argc % 2 != 0) {
        DEBUG_PRINTF("number of arguments not even: %d\n", argc);
        return D_INCORRECT_ARGS;
    }

    if (!is_name_valid(argv[1])) {
        return D_INVALID_NAME;
    }
    ds_init(depotState, argv[1]);

    for (int i = 2; i < argc; i += 2) {
        assert(i + 1 < argc);
        char* matName = argv[i];
        int quantity = parse_int(argv[i + 1]);

        if (!is_name_valid(matName)) {
            return D_INVALID_NAME;
        }
        if (quantity < 0) {
            DEBUG_PRINT("invalid quantity");
            return D_INVALID_QUANTITY;
        }

        ds_alter_mat(depotState, matName, quantity);
    }

    return exec_server(depotState);
}

// starts the program and owns state struct
int main(int argc, char** argv) {
    DepotState depotState = {0};

    DepotExitCode ret = exec_main(argc, argv, &depotState);

    ds_destroy(&depotState);

    fprintf(stderr, "%s", depot_message(ret));
    return ret;
}
