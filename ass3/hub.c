#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "exitCodes.h"
#include "hubState.h"
#include "gameState.h"
#include "deck.h"
#include "util.h"

char** player_args(HubState* hubState, int playerNum, char* name) {
    char** argv = calloc(6, sizeof(char*));
    int n = 0; // because i don't trust myself to count

    argv[n++] = name;
    argv[n++] = int_to_string(hubState->gameState->numPlayers);
    argv[n++] = int_to_string(playerNum);
    argv[n++] = int_to_string(hubState->gameState->threshold);
    
    Deck hand = hubState->playerHands[playerNum];
    argv[n++] = int_to_string(hand.numCards);

    argv[n++] = (char*) NULL; // very important
    assert(n == 6);
    return argv;
}

void exec_child(int fdStdin, int fdStdout, char* name, char** argv) {
    DEBUG_PRINTF("this is the child, pid: %d\n", getpid());

    fflush(stdout);
    fflush(stderr); // for extra safety

    dup2(fdStdin, STDIN_FILENO); // hub's write pipe is our stdin
    dup2(fdStdout, STDOUT_FILENO); // and hub's read pipe is our stdout

#ifndef DEBUG
    // silence stderr if not debugging
    int fdStderr = open("/dev/null", O_CLOEXEC);
    assert(fdStderr != -1);
    dup2(fdStderr, STDERR_FILENO);
    close(fdStderr);
#endif
    // this should never be printed in release mode
    fprintf(stderr, "    stderr after silence\n");

    // close original copies of each file descriptor
    close(fdStdin);
    close(fdStdout);

    errno = 0;
    execv(name, argv); // if successful, will not return
#ifdef DEBUG
    perror(name); // if debugging, print reason for exec failure.
#endif

    // die if exec failed
    _exit(100); // _exit avoids messing with the parent's data and state
}

bool start_player(HubState* hubState, int playerNum, char* name) {
    int fdWrite[2]; // named from OUR (the hub's) perspective!
    int fdRead[2];
    assert(pipe(fdWrite) == 0);
    assert(pipe(fdRead) == 0);

    int forkResult = fork(); // and then there were two
    assert(forkResult >= 0);
    
    // close pipe ends depending on if we're child or not
    int isChild = forkResult == 0 ? 1 : 0;
    close(fdWrite[isChild]); // close write end if we're the child
    close(fdRead[1-isChild]); // close read end if we're the child
    
    if (forkResult == 0) { // this is the CHILD
        char** argv = player_args(hubState, playerNum, name);
        // child's stdin is read end of our write pipe
        // their stdout is write end of our read pipe
        exec_child(fdWrite[0], fdRead[1], name, argv);
        assert(0); // exec_child will NEVER return (EVER!!)
    }

    // wrap in nice FILE* interfaces and don't use fds
    FILE* readFile = fdopen(fdRead[0], "r");
    FILE* writeFile = fdopen(fdWrite[1], "w");
    
    // validate child with @ symbol
    int atSymbol = fgetc(readFile);
    if (atSymbol != '@') {
        DEBUG_PRINTF("no @ received from %d (%s)\n", playerNum, name);
        fclose(readFile);
        fclose(writeFile);
        return false;
    }
    DEBUG_PRINTF("child %d (%s) started. pid: %d\n",
            playerNum, name, forkResult);
    hubState->pipes[playerNum] = (PipePair)
        { .read = readFile, .write = writeFile };
    return true;
}

HubExitCode exec_hub_loop(HubState* hubState) {
    return H_NORMAL;
}

HubExitCode exec_hub_main(int argc, char** argv, HubState* hubState,
        GameState* gameState, Deck* deck) {
    if (argc < 5) {
        return H_INCORRECT_ARGS;
    }
    int threshold = parse_int(argv[2]);
    if (threshold < 2) { // includes error case -1
        return H_INCORRECT_THRESHOLD;
    }
    int numPlayers = argc - 3; // 3 non-player arguments
    char** playerNames = argv + 3; // skip first 3 arguments

    if (!deck_init_file(deck, argv[1])) {
        return H_DECK_ERROR;
    }
    if (deck->numCards < numPlayers) {
        return H_DECK_SHORT;
    }

    gs_init(gameState, numPlayers, threshold);
    hs_init(hubState, gameState);

    hs_deal_cards(hubState, deck);

    DEBUG_PRINTF("hub PID: %d\n", getpid());
    for (int p = 0; p < numPlayers; p++) {
        if (!start_player(hubState, p, playerNames[p])) {
            return H_PLAYER_ERROR;
        }
    }
    for (int p = 0; p < numPlayers; p++) {
    }

    return exec_hub_loop(hubState);
}

int main(int argc, char** argv) {
    // ignore SIGPIPE caused by writes to a dead child
    ignore_sigpipe();

    GameState gameState = {0};
    HubState hubState = {0};
    Deck fullDeck = {0}; // deck of all cards to use

    HubExitCode ret = exec_hub_main(argc, argv,
            &hubState, &gameState, &fullDeck);

    gs_destroy(&gameState);
    hs_destroy(&hubState);
    deck_destroy(&fullDeck);

    print_hub_message(ret);
    DEBUG_PRINTF("exiting hub with code: %d\n", ret);
    return ret;
}
