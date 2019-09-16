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
#include "messages.h"
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
    fprintf(stderr, "    child stderr is visible (DEBUG on)\n");

    // close original copies of each fd because they have been dup2'd
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
        assert(0); // exec_child will NEVER return (EVER!)
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

// returns false on EOF, should always succeed otherwise.
bool send_player_hands(HubState* hubState) {
    MessageStatus status;
    for (int p = 0; p < hubState->gameState->numPlayers; p++) {
        DEBUG_PRINTF("sending HAND to %d\n", p);

        FILE* writeFile = hubState->pipes[p].write;
        Deck hand = hubState->playerHands[p];

        status = msg_send(writeFile, msg_hand(hand));
        if (status != MS_OK) {
            return false;
        }
    }
    return true;
}

/* Broadcasts the given message to all player of the hub, possibly excluding
 * a player whose index is exclude. exclude can be a negative value to send
 * to all players.
 *
 * Returns true on success, false otherwise.
 */
bool broadcast_message(HubState* hubState, Message message, int exclude) {
    MessageStatus status;
    for (int p = 0; p < hubState->gameState->numPlayers; p++) {
        if (p == exclude) {
            continue;
        }
        FILE* file = hubState->pipes[p].write;
        status = msg_send(file, message);
        if (status != MS_OK) {
            return false;
        }
    }
    return true;
}

/* Returns true if the hub should exit upon receiving the given message
 * with given status. If true, stores the appropriate exit code in outCode.
 * If false, does not change outCode.
 *
 * This only returns true if exiting is correct behaviour in all cases. This
 * does not verify the message type is valid (contextually) for a particular
 * caller.
 */
bool hub_should_exit(MessageStatus status, HubExitCode* outCode) {
    if (status != MS_OK) {
        DEBUG_PRINTF("error message status: %d\n", status);
        *outCode = status == MS_EOF ? P_HUB_EOF : P_INVALID_MESSAGE;
        return true;
    }
    return false;
}

HubExitCode exec_player_turn(HubState* hubState, int currPlayer) {
    GameState* gameState = hubState->gameState;
    Message message;
    HubExitCode ret;

    Deck* hand = hubState->playerHands + currPlayer;
    // wait for PLAY from players
    DEBUG_PRINTF("expecting %d PLAY\n", currPlayer);
    FILE* readFile = hubState->pipes[currPlayer].read;
    MessageStatus status = msg_receive(readFile, &message);
    if (hub_should_exit(status, &ret) ||
            message.type != MSG_PLAY_CARD) {
        return ret;
    }
    Card playedCard = message.data.card;

    if (deck_index_of(hand, message.data.card) == -1) {
        DEBUG_PRINT("card not in player's hand");
        return H_INVALID_CARD;
    }

    char leadSuit = hubState->gameState->leadSuit;
    int leadPlayer = hubState->gameState->leadPlayer;
    // if not lead player, and they have a lead suit card but didn't play it
    if (leadPlayer != currPlayer && deck_best_card(hand, leadSuit, true) != -1 &&
            playedCard.suit != leadSuit) {
        DEBUG_PRINT("does not follow lead suit");
        return H_INVALID_CARD;
    }

    DEBUG_PRINT("echoing to other players");
    message = msg_played_card(currPlayer, playedCard);
    if (!broadcast_message(hubState, message, currPlayer)) {
        return H_PLAYER_EOF;
    }
    // gs_play_turn increments currPlayer in the struct
    gs_play_turn(gameState, currPlayer, playedCard);
    hs_played_card(hubState, currPlayer, playedCard);
    return H_NORMAL;
}

HubExitCode exec_hub_loop(HubState* hubState) {
    Message message;
    HubExitCode ret = H_INVALID_MESSAGE;

    // hand size will be the same for all players
    int handSize = hubState->playerHands[0].numCards;
    // one round per card in hand.
    for (int r = 0; r < handSize; r++) {
        int leadPlayer = hubState->gameState->leadPlayer;
        
        // broadcast NEWROUND
        message = msg_new_round(leadPlayer);
        if (!broadcast_message(hubState, message, -1)) {
            return H_PLAYER_EOF;
        }
        // iterate over players
        for (int i = 0; i < hubState->gameState->numPlayers; i++) {
            int currPlayer = hubState->gameState->currPlayer;
            ret = exec_player_turn(hubState, currPlayer);
            if (ret != H_NORMAL) {
                return ret;
            }
        }
    }
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
    // start child players and waits for their @ symbol
    for (int p = 0; p < numPlayers; p++) {
        if (!start_player(hubState, p, playerNames[p])) {
            return H_PLAYER_ERROR;
        }
    }
    // send players their hands.
    if (!send_player_hands(hubState)) {
        return H_PLAYER_EOF;
    }

    // runs main game loop
    return exec_hub_loop(hubState);
}

/* Signal handler for SIGHUP. Exits with the appropriate exit code.
 */
void sighup_handler(int signal) {
    exit(H_SIGNAL); // exit immediately on SIGHUP
}

/* Registers the SIGHUP handler to exit the program.
 */
void register_sighup(void) {
    struct sigaction sa = new_sigaction();
    sa.sa_handler = sighup_handler;
    sigaction(SIGHUP, &sa, NULL);
}

int main(int argc, char** argv) {
    // ignore SIGPIPE caused by writes to a dead child
    ignore_sigpipe();
    // register exit on SIGHUP
    register_sighup();

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
