// xvim: foldmethod=marker:foldlevelstart=0 foldenable

/* includes {{{1 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>

#include "exitCodes.h"
#include "connection.h"
#include "material.h"
#include "depotState.h"
#include "messages.h"
#include "network.h"
#include "util.h"

#define BANNED_NAME_CHARS " \n\r:"

/* type declarations {{{1 */

/* Main function performs basic checks and manages the DepotState.
 * Modifications on the DepotState are performed ONLY by the main thread,
 * messages are received via a single incoming message channel which is used
 * for regular depot messages, connection state changes and signals.
 *
 * Threads are initialised for receiving signals and starting new connections.
 *
 * When a connection is connected, a reader_thread is started to send and 
 * receive IM messages. If successful, this sends a MSG_META_CONN_NEW down the
 * incoming channel and it starts processing messages from that socket.
 * Writing to sockets is done exclusively by the main thread.
 *
 * In general, functions which take a DepotState parameter should ONLY be
 * called by the main thread.
 */

// struct for passing information into server_thread
typedef struct ServerData {
    int ourPort; // this depot's port
    char* ourName; // BORROWED this depot's name
    int fd; // file descriptor of the server
    Channel* incoming; // BORROWED incoming message channel
} ServerData;

// struct for passing data into reader_thread. this owns the FILE*'s but if
// verification is successful, YIELDS files back to main thread through a
// Connection struct.
typedef struct ReaderData {
    int ourPort;
    char* ourName; // BORROW

    FILE* readFile; // OWNED
    FILE* writeFile; // OWNED
    Channel* incoming; // BORROW
} ReaderData;

// see implementation
void start_reader_thread(int port, char* name, Channel* incoming, int fd);
// see implementaiton
void execute_message(DepotState* depotState, Message* message);

/* util methods {{{1 */

/* Constructs and returns a set of signals which should be blocked and picked
 * up via sigwait. Contains at least SIGHUP.
 */
sigset_t blocked_sigset(void) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGHUP);
    //sigaddset(&ss, SIGUSR1);
    return sigset;
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

/* Returns whether the givne material is a valid message argument. 
 * That is, its name is valid and its quantity is strictly positive.
 */
bool is_mat_valid(Material material) {
    if (!is_name_valid(material.name)) {
        DEBUG_PRINT("invalid mat name");
        return false;
    }
    if (material.quantity <= 0) {
        DEBUG_PRINT("invalid mat quantity");
        return false;
    }
    return true;
}

/* Returns true if a connection to the given port exists, false otherwise.
 * If the given port is this depot's port, returns true.
 */
bool is_port_connected(DepotState* depotState, int port) {
    if (port == depotState->port) {
        DEBUG_PRINT("rejecting connection to our own port");
        return true;
    }
    bool portExists = false;
    for (int i = 0; i < depotState->connections->numItems; i++) {
        if (ARRAY_ITEM(Connection, depotState->connections, i)->port
                == port) {
            DEBUG_PRINT("active connection already exists");
            portExists = true;
            break;
        }
    }
    for (int i = 0; i < depotState->pending->numItems; i++) {
        if (*(int*)array_get_at(depotState->pending, i) == port) {
            DEBUG_PRINT("unverified connection already exists");
            portExists = true;
            break;
        }
    }
    return portExists;
}

/* execute methods {{{1 */
/* execute normal messages {{{2 */

/* The execute family of functions will each execute a particular message type,
 * given the depot state and message instance. Particular execute_ functions
 * should only by called with the correct tyle. execute_message and
 * execute_meta_message can be called with any message of normal / meta type
 * respectively. Because invalid messages are silently ignored, these functions
 * do not return anything.
 */

// executes a Connect:port message
void execute_connect(DepotState* depotState, Message* message) {
    int portNum = message->data.depotPort;
    if (is_port_connected(depotState, portNum)) {
        DEBUG_PRINTF("connection exists to port %d\n", portNum);
        return;
    }

    char* port = asprintf("%d", portNum);
    DEBUG_PRINTF("trying to connect to port %s\n", port);
    int fd;
    bool started = start_active_socket(&fd, port);
    free(port);

    if (started) {
        DEBUG_PRINT("connection established, verifying...");
        array_add_copy(depotState->pending, &portNum, sizeof(int));
        start_reader_thread(depotState->port, depotState->name, 
                depotState->incoming, fd);
    } else {
        DEBUG_PRINT("failed to connect");
    }
}

// executes a Deliver or Withdraw message
void execute_deliver_withdraw(DepotState* depotState, Message* message) {
    if (!is_mat_valid(message->data.material)) {
        DEBUG_PRINT("ignoring invalid material");
        return;
    }

    int delta = message->data.material.quantity;
    if (message->type != MSG_DELIVER) {
        delta = -delta; // negative change to represent withdraw
    }
    char* name = message->data.material.name;

    ds_alter_mat(depotState, name, delta);
}

// executes a Transfer message. writes to a socket file!
void execute_transfer(DepotState* depotState, Message* message) {
    Material mat = message->data.material;
    if (!is_mat_valid(mat)) {
        DEBUG_PRINT("ignoring invalid material");
        return;
    }

    Connection* conn = arraymap_get(depotState->connections,
            message->data.depotName);
    if (conn == NULL) {
        DEBUG_PRINTF("depot not found: %s\n", message->data.depotName);
        return;
    }

    DEBUG_PRINTF("withdrawing, then delivering to %s\n", conn->name);
    ds_alter_mat(depotState, mat.name, -mat.quantity);

    Message msg = msg_deliver(mat.quantity, mat.name);
    msg_send(conn->writeFile, msg); // send Deliver to given depot
    msg_destroy(&msg);
}

// executes a Defer message. only deliver, withdraw and transfer messages can
// be deferred
void execute_defer(DepotState* depotState, Message* message) {
    Message* deferMessage = message->data.deferMessage;

    // restrict types of messages which can be deferred
    MessageType deferType = deferMessage->type;
    if (!(deferType == MSG_DELIVER || deferType == MSG_WITHDRAW ||
            deferType == MSG_TRANSFER)) {
        DEBUG_PRINT("unsupported deferred message type");
        return; // silently ignore
    }

    DeferGroup* dg = ds_ensure_defer_group(depotState, message->data.deferKey);
    dg_add_message(dg, message->data.deferMessage);
    // deferMessage now owned by defer group. delete our reference.
    message->data.deferMessage = NULL;
    // we can destroy message now because the sub-message has been copied
    // and its reference in *message is set to NULL.
}

// executes an Execute message
void execute_execute(DepotState* depotState, Message* message) {
    // admittedly not the best naming

    DeferGroup* dg = arraymap_get(depotState->deferGroups, 
            &message->data.deferKey);
    if (dg == NULL) {
        DEBUG_PRINT("defer key not found");
        return;
    }
    DEBUG_PRINTF("executing defer group, key: %d\n", message->data.deferKey);

    for (int i = 0; i < dg->messages->numItems; i++) {
        DEBUG_PRINTF("executing deferred message %d\n", i);
        Message* msg = ARRAY_ITEM(Message, dg->messages, i);
        msg_debug(msg);
        execute_message(depotState, msg); // recursion!
    }

    // frees messages and destroys messages array
    dg_destroy(dg);

    // remove defer group from array of defer groups
    array_remove(depotState->deferGroups, dg);
    // and free memory
    free(dg);
}

/* }}}2 */

// executes an arbitrary normal message (those defined in spec)
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
            DEBUG_PRINT("invalid normal message type");
            assert(0);
    }
}

// executes a single METE message. these messages affect the state of
// connections / signals of the depot, not the actual materials
void execute_meta_message(DepotState* depotState, Message* message) {
    Connection* conn = message->data.connection;
    int signal = message->data.signal;
    switch (message->type) {
        case MSG_META_SIGNAL:
            DEBUG_PRINTF("received signal %d: %s\n", signal,
                    strsignal(signal));
            if (signal == SIGHUP) {
                ds_print_info(depotState);
            }
            break;
        case MSG_META_CONN_NEW:
            DEBUG_PRINTF("new connection to %d:%s, %p\n", conn->port,
                    conn->name, (void*)conn);
            int* pendingPort = arraymap_get(depotState->pending, &conn->port);
            if (pendingPort != NULL) {
                DEBUG_PRINTF("connection verified on port %d\n", conn->port);
                array_remove(depotState->pending, pendingPort);
                free(pendingPort);
            }
            if (arraymap_get(depotState->connections, conn->name) != NULL ||
                    is_port_connected(depotState, conn->port)) {
                DEBUG_PRINT("connection to name or port exists, ignoring.");
                break; // main thread will cleanup
            }

            DEBUG_PRINT("accepting new connection");
            // YIELD connection to connections array
            array_add(depotState->connections, conn);
            arraymap_sort(depotState->connections);
            message->data.connection = NULL; // don't destroy conn
            // start reader thread to get incoming messages
            //start_reader_thread(conn, depotState->incoming);
            break;
        case MSG_META_CONN_EOF:
            DEBUG_PRINTF("conn %d:%s, %p LOST connection!\n", conn->port,
                    conn->name, (void*)conn);
            //array_remove(depotState->connections, conn);
            // edit: as of 4.2, do not remove closed connections. keep FILES's
            // open, will fail on writing.
            message->data.connection = NULL; // don't destroy conn
            break;
        default:
            DEBUG_PRINT("invalid meta message type");
            assert(0);
    }
}

/* reader/writer threads {{{1 */

/* Parses messages from the given conn (as a Connection*) and writes the
 * messages into the given incoming Channel. Returns on EOF of read file.
 */
void reader_thread_loop(Connection* connection, Channel* incoming) {
    Connection* conn = connection;
    DEBUG_PRINTF("reader loop started for %d:%s\n", conn->port, conn->name);

    MessageStatus status = MS_OK;
    while (status != MS_EOF) { // loop until EOF
        Message msg = {0};
        status = msg_receive(conn->readFile, &msg);
        if (status != MS_OK) {
            DEBUG_PRINT("message invalid or eof, continuing");
            continue;
        }
        // wrap received message in a heap-allocated struct
        Message* msgNew = calloc(1, sizeof(Message));
        *msgNew = msg;
        DEBUG_PRINTF("posting to incoming channel: %p\n", (void*)msgNew);
        // YIELD received message to channel
        chan_post(incoming, msgNew);
        DEBUG_PRINTF("message posted: %p\n", (void*)msgNew);
    }
    DEBUG_PRINTF("reader reached EOF for %d:%s\n", conn->port, conn->name);
}

/* Verifies the connection by sending and receiving IM messages, using files
 * inside readerData. If successful, returns true and stores the received IM
 * into *outMessage, otherwise returns false.
 */
bool verify_connection(ReaderData* readerData, Message* outMessage) {
    Message msg = msg_im(readerData->ourPort, readerData->ourName);
    if (msg_send(readerData->writeFile, msg) != MS_OK) {
        DEBUG_PRINT("sending IM failed. closing");
        msg_destroy(&msg);
        return false;
    }
    msg_destroy(&msg); // destroy the msg_im()
    if (msg_receive(readerData->readFile, &msg) != MS_OK ||
            msg.type != MSG_IM || !is_name_valid(msg.data.depotName)) {
        DEBUG_PRINT("invalid IM or bad depot name. closing.");
        msg_destroy(&msg);
        return false;
    }
    *outMessage = msg;
    return true;
}

/* Worker thread for establishing and maintaining communication with one
 * socket. Accepts argument of ReaderData* cast to void*, return value unused.
 * Verifies connection with IM, then sends MSG_META_CONN_NEW, then reads
 * incoming messages, then sends MSG_META_CONN_EOF. Returns when the connection
 * is closed.
 */
void* reader_thread(void* readerArg) {
    ReaderData readerData = *(ReaderData*)readerArg;
    free(readerArg);
    DEBUG_PRINT("verifying connection");
    // prevents becoming a thread zombie. success is indicated to main via
    // channel. on failure, thread ends silently.
    pthread_detach(pthread_self());

    Message msg;
    if (!verify_connection(&readerData, &msg)) {
        DEBUG_PRINT("acknowledge failed");
        fclose(readerData.readFile);
        fclose(readerData.writeFile);
        return NULL;
    }
    Connection* conn = calloc(1, sizeof(Connection));
    conn_init(conn, msg.data.depotPort, msg.data.depotName);
    conn_set_files(conn, readerData.readFile, readerData.writeFile);
    DEBUG_PRINTF("acknowledged by %s on %d\n", conn->name, conn->port);
    msg_destroy(&msg); // copied into conneciton

    // inform main of the new connection
    msg = (Message) {0};
    msg.type = MSG_META_CONN_NEW;
    // YIELDS connection struct and contained files to main thread.
    msg.data.connection = conn;
    Message* msgNew = malloc(sizeof(Message));
    *msgNew = msg;
    chan_post(readerData.incoming, msgNew);

    // loop and post incoming messages down channel
    reader_thread_loop(conn, readerData.incoming);

    // send meta eof message to managing thread.
    msg.type = MSG_META_CONN_EOF; // keep msg.data.connection from previous msg
    msgNew = malloc(sizeof(Message));
    *msgNew = msg;
    chan_post(readerData.incoming, msgNew);
    return NULL;
}

/* Starts a reader thread which communicates with the given fd.
 * Also takes port and name of THIS depot to send in IM message, and channel
 * to send MSG_META_CONN_NEW to.
 */
void start_reader_thread(int port, char* name, Channel* incoming, int fd) {
    ReaderData* readerData = malloc(sizeof(ReaderData));
    readerData->ourPort = port;
    readerData->ourName = name;
    readerData->incoming = incoming;

    int fdRead = fd;
    int fdWrite = dup(fd);
    readerData->readFile = fdopen(fdRead, "r");
    readerData->writeFile = fdopen(fdWrite, "w");

    pthread_t readerThread;
    pthread_create(&readerThread, NULL, reader_thread, readerData);
}

/* server / signal threads {{{1 */

/* Thread which listens passively for incoming connections, starting a verify
 * thread for each new connection. Argument contains fd of listening socket,
 * port and name of this depot and incoming channel. Return value unused.
 */
void* server_thread(void* serverArg) {
    ServerData serverData = *(ServerData*)serverArg;
    free(serverArg);
    DEBUG_PRINT("server thread started");

    while (1) {
        // listen for connections
        int fd = accept(serverData.fd, 0, 0);
        DEBUG_PRINT("server got new connection, verifying...");
        start_reader_thread(serverData.ourPort, serverData.ourName,
                serverData.incoming, fd);
    }
    assert(0);
}

/* Starts a new server_thread with the given depot state and listening on the
 * given fd. Returns TID of server thread.
 */
pthread_t start_server_thread(DepotState* depotState, int fdServer) {
    // start server listener thread. data argument is allocated on stack!
    ServerData* serverData = malloc(sizeof(ServerData));
    serverData->fd = fdServer;
    serverData->ourName = depotState->name;
    serverData->ourPort = depotState->port;
    serverData->incoming = depotState->incoming;

    pthread_t serverThread;
    pthread_create(&serverThread, NULL, server_thread, serverData);
    return serverThread;
}

/* Signal thread to wait for signals specified in blocked_sigset(). For each
 * signal received, sends a MSG_META_SIGNAL to the given incoming Channel*,
 * passed as the sole argument. Return value unused.
 */
void* signal_thread(void* signalArg) {
    Channel* incoming = signalArg;
    sigset_t sigset = blocked_sigset();

    while (1) {
        int sig; // 'signal' is the name of a function
        sigwait(&sigset, &sig);
        DEBUG_PRINTF("signal %d: %s\n", sig, strsignal(sig));
        Message* msg = calloc(1, sizeof(Message));
        msg->type = MSG_META_SIGNAL;
        msg->data.signal = sig;
        // post to incoming messages channel
        chan_post(incoming, msg);
    }
}

/* main functions {{{1*/

/* Starts and runs the main loop of the depot. Starts server and signal threads
 * and processes all messages arriving on the depot state's incoming messages
 * channel. 
 *
 * This thread has exclusive ownership of the DepotState. Should never return
 * in normal execution but always returns D_NORMAL.
 */
DepotExitCode exec_depot_loop(DepotState* depotState) {
    DEBUG_PRINT("starting depot");
    // block SIGHUP from all threads. we will pick it up via sigwait
    sigset_t ss = blocked_sigset();
    pthread_sigmask(SIG_BLOCK, &ss, NULL);
    ignore_sigpipe(); // also, ignore SIGPIPE completely

    // start this server thing
    int server;
    int port;
    if (!start_passive_socket(&server, &port)) {
        DEBUG_PRINT("failed to start passive socket");
        return D_NORMAL; // no special exit code
    }
    depotState->port = port; // store our port number

    // start thread to listen for signals
    pthread_t signalThread;
    pthread_create(&signalThread, NULL, signal_thread, depotState->incoming);
    // start server to listen for incoming connections
    start_server_thread(depotState, server);

    // IMPORTANT: print port number after listen and signal threads started
    printf("%d\n", port);
    fflush(stdout);

    // main loop of the depot. acts on incoming messages
    while (1) {
        Message* msg = chan_wait(depotState->incoming);
        int numItems;
        sem_getvalue(&depotState->incoming->numItems, &numItems);
        DEBUG_PRINTF("received %s message, %d messages remain\n", 
                msg_code(msg->type), numItems);
        if (msg->type >= MSG_NULL) { // meta messages are after MSG_NULL
            execute_meta_message(depotState, msg);
        } else {
            execute_message(depotState, msg);
        }
        msg_destroy(msg);
        free(msg);
    }
    assert(0); // should never get here
    return D_NORMAL;
}

/* Does argument checks and initialises depot state. Executes server 
 * listener.
 */
DepotExitCode exec_main(int argc, char** argv, DepotState* depotState) {
    if (argc % 2 != 0) {
        DEBUG_PRINTF("number of arguments not even: %d\n", argc);
        return D_INCORRECT_ARGS;
    }

    if (!is_name_valid(argv[1])) {
        return D_INVALID_NAME;
    }
    ds_init(depotState, argv[1]); // depotState BORROWS argv[1]
    // first 2 args are program name and depot name. iterate in steps of 2
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
        // has side effect of summing repeated materials
        ds_alter_mat(depotState, matName, +quantity);
    }

    return exec_depot_loop(depotState);
}

// starts the program and owns state struct
int main(int argc, char** argv) {
    DepotState depotState = {0};

    DepotExitCode ret = exec_main(argc, argv, &depotState);

    ds_destroy(&depotState);

    fprintf(stderr, "%s", depot_message(ret));
    DEBUG_PRINTF("program exiting with code: %d\n", ret);
    return ret;
}
