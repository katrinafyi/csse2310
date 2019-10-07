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
#include <arpa/inet.h>


#include "exitCodes.h"
#include "connection.h"
#include "material.h"
#include "depotState.h"
#include "messages.h"
#include "util.h"

#define BANNED_NAME_CHARS " \n\r:"
#define CONNECTION_QUEUE 32

// normally, i avoid declaring functions in .c files but the control flow
// here is particularly convoluted.
// see implementations for comments.
void start_connection_thread(DepotState* depotState, int fd);
// see implementation.
bool start_active_socket(int* fdOut, char* port);
// see implementaiton
void execute_message(DepotState* depotState, Message* message);


typedef struct ThreadData {
    int fd;
    DepotState* depotState;
} ThreadData;


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


void execute_connect(DepotState* depotState, Message* message) {
    char* port = asprintf("%d", message->data.depotPort);
    int fd;
    bool started = start_active_socket(&fd, port);
    free(port);
    if (started) {
        start_connection_thread(depotState, fd);
    }
}

void execute_deliver_withdraw(DepotState* depotState, Message* message) {

    int delta = message->data.material.quantity;
    if (message->type != MSG_DELIVER) {
        delta = -delta; // negative change to represent withdraw
    }
    char* name = message->data.material.name;
    if (!is_name_valid(name)) {
        DEBUG_PRINT("ignoring invalid material name");
        return;
    }

    // need write lock because we have no individual locks per material
    // and this could also add a new material
    ARRAY_WRLOCK(depotState->materials);
    ds_alter_mat(depotState, name, delta);
    ARRAY_UNLOCK(depotState->materials);
}

void execute_transfer(DepotState* depotState, Message* message) {
    Material mat = message->data.material;
    if (!is_name_valid(mat.name)) {
        DEBUG_PRINT("ignoring invalid material name");
        return;
    }

    ARRAY_RDLOCK(depotState->connections);
    Connection* conn = arraymap_get(depotState->connections,
            message->data.depotName);
    if (conn == NULL) {
        DEBUG_PRINTF("depot not found: %s\n", message->data.depotName);
        ARRAY_UNLOCK(depotState->connections);
        return;
    }

    DEBUG_PRINTF("withdrawing, then delivering to %s\n", conn->name);
    ARRAY_WRLOCK(depotState->materials);
    ds_alter_mat(depotState, mat.name, -mat.quantity);
    ARRAY_UNLOCK(depotState->materials);

    // ignore return value. msg_deliver doesn't need to be destroyed
    msg_send(conn->writeFile, msg_deliver(mat.quantity, mat.name));

    ARRAY_UNLOCK(depotState->connections);
}

void execute_defer(DepotState* depotState, Message* message) {
    Message* deferMessage = message->data.deferMessage;
    switch (deferMessage->type) {
        case MSG_DELIVER:
        case MSG_WITHDRAW:
        case MSG_TRANSFER:
            break; // only these message types are valid for Defer
        default:
            DEBUG_PRINT("unsupported deferred message type");
            msg_destroy(message);
            return; // silently ignore
    }
    ARRAY_RDLOCK(depotState->deferGroups);
    DeferGroup* dg = ds_ensure_defer_group(depotState, message->data.deferKey);
    
    ARRAY_WRLOCK(dg->messages);
    dg_add_message(dg, *message->data.deferMessage);
    // message moved into new location in array. free original and set to null
    TRY_FREE(message->data.deferMessage);
    ARRAY_UNLOCK(dg->messages);

    ARRAY_UNLOCK(depotState->deferGroups);

    // we can destroy message now because the sub-message has been copied
    // and its reference in *message is set to NULL.
}

void execute_execute(DepotState* depotState, Message* message) {
    // admittedly not the best naming

    ARRAY_WRLOCK(depotState->deferGroups);
    DeferGroup* dg = arraymap_get(depotState->deferGroups, 
            &message->data.deferKey);
    if (dg == NULL) {
        DEBUG_PRINT("defer key not found");
        ARRAY_UNLOCK(depotState->deferGroups);
        return;
    }

    // don't need to lock on defer group because we have an exclusive write
    // lock on the entire array of defer groups
    for (int i = 0; i < dg->messages->numItems; i++) {
        DEBUG_PRINTF("executing deferred message %d\n", i);
        Message* msg = ARRAY_ITEM(Message, dg->messages, i);
        msg_debug(msg);
        execute_message(depotState, msg);
    }

    // frees messages and destroys messages array
    dg_destroy(dg);

    // remove defer group from array of defer groups
    array_remove(depotState->deferGroups, dg);
    // and free memory
    free(dg);

    ARRAY_UNLOCK(depotState->deferGroups);
}

void execute_message(DepotState* depotState, Message* message) {
    switch (message->type) {
        case MSG_CONNECT:
            execute_connect(depotState, message);
            break;
        case MSG_IM: // ignore improperly sequenced IM
            DEBUG_PRINT("ignoring unexpected IM");
            break;
        case MSG_DELIVER:
        case MSG_WITHDRAW:
            execute_deliver_withdraw(depotState, message);
            break;
        case MSG_TRANSFER:
            execute_transfer(depotState, message);
            break;
        case MSG_DEFER:
            execute_defer(depotState, message);
            break;
        case MSG_EXECUTE:
            execute_execute(depotState, message);
            break;
        default:
            assert(0);
    }
}

Connection* init_connection(DepotState* depotState, FILE* readFile, 
        FILE* writeFile) {
    Message msg = msg_im(depotState->port, depotState->name);
    if (msg_send(writeFile, msg) != MS_OK) {
        DEBUG_PRINT("sending IM failed");
        return NULL;
    }
    msg = (Message) {0}; // msg_im() must not be msg_destroy()'d
    if (msg_receive(readFile, &msg) != MS_OK || msg.type != MSG_IM ||
            !is_name_valid(msg.data.depotName)) {
        DEBUG_PRINT("invalid IM");
        msg_destroy(&msg);
        return NULL;
    }

    // add to list of connections
    ARRAY_WRLOCK(depotState->connections);
    Connection* conn = ds_add_connection(depotState, msg.data.depotPort,
            msg.data.depotName, readFile, writeFile);
    conn->thread = pthread_self(); // store current thread ID
    msg_destroy(&msg); // destroy received message
    ARRAY_UNLOCK(depotState->connections);

    DEBUG_PRINTF("acknowledged by %s on %d\n", conn->name, conn->port);

    return conn;
}

void* connection_thread(void* threadDataArg) {
    ThreadData* threadData = threadDataArg;
    DepotState* depotState = threadData->depotState;
    int fdRead = threadData->fd;
    int fdWrite = dup(fdRead);
    pthread_t thread = pthread_self();

    free(threadDataArg); // data has been copied into local variables.
    DEBUG_PRINT("connection thread started");
    FILE* readFile = fdopen(fdRead, "r");
    FILE* writeFile = fdopen(fdWrite, "w");
    // initialise connection by sending and receiving IM
    Connection* conn = init_connection(depotState, readFile, writeFile);
    if (conn == NULL) {
        DEBUG_PRINT("connection acknowledge failed, terminating thread.");
        fclose(readFile);
        fclose(writeFile);
        // detach thread to cleanup its resources automatically
        pthread_detach(thread);
        return NULL;
    }

    // main loop of this connection
    Message msg = {0};
    MessageStatus status = MS_OK;
    while (status != MS_EOF) { // break on EOF
        status = msg_receive(readFile, &msg);
        if (status != MS_OK) {
            DEBUG_PRINT("message invalid or EOF");
            continue; // ignore invalid messages
        }
        execute_message(depotState, &msg);
        msg_destroy(&msg); // finished with this message
    }

    DEBUG_PRINTF("thread closing normally, name: %s\n", conn->name);
    // destroy connection and remove from list of connections
    ARRAY_WRLOCK(depotState->connections);
    conn_destroy(conn); // closes files for us
    array_remove(depotState->connections, conn); // remove from array
    free(conn); // because array stored a copy of the connection
    pthread_detach(thread); // detach thread to free resources on termination
    ARRAY_UNLOCK(depotState->connections);

    return NULL;
}

void start_connection_thread(DepotState* depotState, int fd) {
    ThreadData* arg = malloc(sizeof(ThreadData));
    arg->fd = fd;
    arg->depotState = depotState;

    pthread_t thread;
    pthread_create(&thread, NULL, connection_thread, arg);
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
    *aiOut = ai;

    if (error != 0) {
        DEBUG_PRINTF("getaddrinfo: %s\n", gai_strerror(error));
        return false;
    }
    *fdOut = socket(AF_INET, SOCK_STREAM, 0); // 0 is default protocol
    return true;
}

bool start_active_socket(int* fdOut, char* port) {
    struct addrinfo* ai;
    int sock;
    // init new socket()
    if (!new_socket(port, &sock, &ai)) {
        return false;
    }
    if (connect(sock, ai->ai_addr, ai->ai_addrlen) != 0) {
        DEBUG_PRINT("connect failed");
        freeaddrinfo(ai);
        return false;
    }
    freeaddrinfo(ai);
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
    if (bind(server, ai->ai_addr, ai->ai_addrlen) != 0) {
        freeaddrinfo(ai);
        DEBUG_PRINTF("bind failed: %s\n", strerror(errno));
        return false;
    }
    freeaddrinfo(ai);
    ai = NULL;

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

void* server_thread(void* depotStateArg) {
    DepotState* depotState = depotStateArg;
    DEBUG_PRINT("server thread started");

    int server;
    int port;
    // start server socket in listen mode
    if (!start_passive_socket(&server, &port)) {
        DEBUG_PRINT("failed to start passive socket");
        return NULL; // no special exit code
    }
    depotState->port = port;
    printf("%d\n", port);

    while (1) {
        // listen for connections
        int fd = accept(server, 0, 0);
        DEBUG_PRINT("accepted connection");
        start_connection_thread(depotState, fd);
    }
    assert(0);
}

void print_depot_info(DepotState* depotState) {
    ARRAY_RDLOCK(depotState->materials);
    printf("Goods:\n");
    for (int i = 0; i < depotState->materials->numItems; i++) {
        Material* mat = ARRAY_ITEM(Material, depotState->materials, i);
        printf("%s %d\n", mat->name, mat->quantity);
    }
    ARRAY_UNLOCK(depotState->materials);

    ARRAY_RDLOCK(depotState->connections);
    printf("Neighbours:\n");
    for (int i = 0; i < depotState->connections->numItems; i++) {
        Connection* conn = ARRAY_ITEM(Connection, depotState->connections, i);
        printf("%s\n", conn->name);
    }
    ARRAY_UNLOCK(depotState->connections);
}

DepotExitCode exec_server(DepotState* depotState) {
    DEBUG_PRINT("starting server");

    // block SIGHUP from all threads. we will pick it up via sigwait
    static sigset_t ss;
    sigemptyset(&ss);
    sigaddset(&ss, SIGHUP);
    sigaddset(&ss, SIGUSR1); // use SIGUSR1 to exit cleanly
    pthread_sigmask(SIG_BLOCK, &ss, NULL);

    // start server listener thread
    pthread_t serverThread; // unused
    pthread_create(&serverThread, NULL, server_thread, depotState);

    // this thread handles the signals
    while (1) {
        int sig;
        sigwait(&ss, &sig);
        DEBUG_PRINTF("signal %d: %s\n", sig, strsignal(sig));

        if (sig != SIGHUP) {
            break; // terminate if caught non SIGHUP signal
        }
        print_depot_info(depotState);
    }
    // terminate the program
    DEBUG_PRINT("terminating program due to signal!");

    ARRAY_WRLOCK(depotState->connections);
    // terminate all threads. stop server first to prevent new connections
    pthread_cancel(serverThread);
    pthread_join(serverThread, NULL);
    for (int i = 0; i < depotState->connections->numItems; i++) {
        Connection* conn = ARRAY_ITEM(Connection, depotState->connections, i);
        pthread_cancel(conn->thread);
        pthread_join(conn->thread, NULL);
    }
    // at this point, we are back to a single thread
    ARRAY_UNLOCK(depotState->connections);
    return D_NORMAL;
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
