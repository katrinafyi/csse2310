#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "exitCodes.h"
#include "hubState.h"
#include "gameState.h"
#include "messages.h"
#include "deck.h"
#include "util.h"

// global state variable for signal handlers to kill children.
HubState* hubStateGlobal;

/* Determines and returns the appropriate args to start a child with the given
 * hubState, player number and executable file name.
 *
 * Returns a MALLOC'd array of strings.
 */
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

/* This is called in the child fork to intitialise all the input/output and
 * exec the given player program.
 *
 * Takes arguments of file descriptors to use as child's stdin, stdout, and
 * child's name and arguments to send to exec.
 * Will NEVER return.
 */
void exec_child(int fdStdin, int fdStdout, char* name, char** argv) {
    noop_printf("this is the child, pid: %d\n", getpid());
    fflush(stdout);
    fflush(stderr); // for extra safety

    dup2(fdStdin, STDIN_FILENO); // hub's write pipe is our stdin
    dup2(fdStdout, STDOUT_FILENO); // and hub's read pipe is our stdout

#ifndef DEBUG
    // silence stderr if not debugging
    int fdStderr = open("/dev/null", O_WRONLY); // open for write
    assert(fdStderr != -1);
    dup2(fdStderr, STDERR_FILENO);
    close(fdStderr);
#endif
    // this should never be printed in release mode which is why it is
    // left in.
    fprintf(stderr, TERM_RED "        warning: "
            TERM_RESET "child stderr (DEBUG on)\n");

    // close original copies of each fd because they have been dup2'd
    close(fdStdin);
    close(fdStdout);

    errno = 0;
    execvp(name, argv); // if successful, will not return
    noop_printf("execv failed (%s): %s\n", name, strerror(errno));

    // die if exec failed. hub will detect missing @
    // _exit avoids messing with the parent's data and state
    _exit(100);
}

/* This is called by the hub to prepare pipes for communication with the chlid
 * and forks to spawn a new process. Also ensures that @ is received from the
 * player.
 *
 * Arguments of hub state, this player's index and this player's file name.
 * Returns true if child started successfully, false otherwise.
 */
bool start_player(HubState* hubState, int playerNum, char* name) {
    int fdWrite[2]; // named from OUR (the hub's) perspective!
    int fdRead[2];
    if (pipe(fdWrite) != 0 || pipe(fdRead) != 0) {
        return false; // making a pipe failed.
    }

    int forkResult = fork(); // and then there were two
    if (forkResult < 0) {
        return false; // fork failed
    }
    
    // close pipe ends depending on if we're child or not
    int isChild = forkResult == 0 ? 1 : 0;
    close(fdWrite[isChild]); // close write end if we're the child
    close(fdRead[1 - isChild]); // close read end if we're the child
    
    if (forkResult == 0) { // this is the CHILD
        // warning: player_args leaks memory all over the place which we can
        // never free after calling exec :,(
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
        noop_printf("no @ received from %d (%s)\n", playerNum, name);
        fclose(readFile);
        fclose(writeFile);
        return false;
    }
    noop_printf("child %d (%s) started. pid: %d (%c)\n",
            playerNum, name, forkResult, PID_CHAR(forkResult));
    // store the pipe
    hs_add_player(hubState, playerNum, forkResult, readFile, writeFile);
    return true;
}

/* Sends each player their hands, from the hands and players in hubState.
 * Returns true on success, false on error.
 */
bool send_player_hands(HubState* hubState) {
    MessageStatus status;
    for (int p = 0; p < hubState->gameState->numPlayers; p++) {
        noop_printf("sending HAND to %d\n", p);

        FILE* writeFile = hubState->pipes[p].write;
        Deck hand = hubState->playerHands[p];

        status = msg_send(writeFile, msg_hand(hand));
        if (status != MS_OK) {
            return false;
        }
    }
    return true;
}

/* Broadcasts the given message to all players of the hub, possibly excluding
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
 * status. If true, stores the appropriate exit code in outCode.
 * If false, does not change outCode.
 */
bool hub_should_exit(MessageStatus status, HubExitCode* outCode) {
    if (status != MS_OK) {
        noop_printf("hub error message status: %d\n", status);
        *outCode = status == MS_EOF ? H_PLAYER_EOF : H_INVALID_MESSAGE;
        return true;
    }
    return false;
}

/* Runs one player's turn, with the player number given by currPlayer.
 * Ensures the card they play is valid and also echos it to all other players.
 */
HubExitCode one_player_turn(HubState* hubState, int currPlayer) {
    GameState* gameState = hubState->gameState;
    Message message;
    HubExitCode ret = H_INVALID_CARD;

    Deck* hand = hubState->playerHands + currPlayer;
    // wait for PLAY from players
    noop_printf("hub expecting %d to PLAY\n", currPlayer);
    FILE* readFile = hubState->pipes[currPlayer].read;
    MessageStatus status = msg_receive(readFile, &message);
    if (hub_should_exit(status, &ret) ||
            message.type != MSG_PLAY_CARD) {
        return ret;
    }
    Card playedCard = message.data.card; // store played card

    if (deck_index_of(hand, message.data.card) == -1) {
        noop_print("card not in player's hand");
        return H_INVALID_CARD;
    }

    char leadSuit = hubState->gameState->leadSuit;
    int leadPlayer = hubState->gameState->leadPlayer;
    bool violatesSuit = (deck_best_card(hand, leadSuit, true) != -1 &&
            playedCard.suit != leadSuit);
    // if not lead player, and they have a lead suit card but didn't play it
    if (leadPlayer != currPlayer && violatesSuit) {
        noop_print("does not follow lead suit");
        return H_INVALID_CARD;
    }
    // send PLAYED to other players excluding this one
    noop_print("echoing to other players");
    message = msg_played_card(currPlayer, playedCard);
    if (!broadcast_message(hubState, message, currPlayer)) {
        return H_PLAYER_EOF;
    }
    // gs_card_played increments currPlayer in the struct
    gs_card_played(gameState, currPlayer, playedCard);
    hs_card_played(hubState, currPlayer, playedCard);
    return H_NORMAL;
}

/* Prints the cards played in this round, in the order they were played, to
 * stdout.
 */
void print_round_cards(HubState* hubState) {
    GameState* gameState = hubState->gameState;

    // prints message of cards this round, in order of play
    printf("Cards=");
    gs_fprint_cards(gameState, stdout);
}

/* Computes and prints player scores to stdout, considering threshold and
 * points won.
 */
void print_player_scores(HubState* hubState) {
    GameState* gameState = hubState->gameState;
    int threshold = gameState->threshold;

    for (int p = 0; p < gameState->numPlayers; p++) {
        if (p > 0) {
            printf(" ");
        }
        int points = gameState->playerPoints[p];
        int diamonds = gameState->diamondsWon[p];
        int diamondsMult = diamonds >= threshold ? 1 : -1;

        int total = points + diamondsMult * diamonds;
        printf("%d:%d", p, total);
    }
    printf("\n");
}

/* Executes the main game loop of the hub, running all the rounds and managing
 * players. Prints cards at end of each round and scores at end of game.
 *
 * Does not broadcast GAMEOVER!
 */
HubExitCode exec_hub_loop(HubState* hubState) {
    GameState* gameState = hubState->gameState;
    Message message;
    HubExitCode ret = H_INVALID_MESSAGE;

    int numPlayers = hubState->gameState->numPlayers;
    // hand size will be the same for all players
    int handSize = hubState->playerHands[0].numCards;
    // one round per card in hand.
    for (int r = 0; r < handSize; r++) {
        int leadPlayer = hubState->gameState->leadPlayer;
        printf("Lead player=%d\n", leadPlayer);
        gs_new_round(gameState, leadPlayer);
        
        // broadcast NEWROUND
        message = msg_new_round(leadPlayer);
        if (!broadcast_message(hubState, message, -1)) {
            return H_PLAYER_EOF;
        }
        // iterate over players, running their turn
        for (int i = 0; i < numPlayers; i++) {
            int currPlayer = gameState->currPlayer;
            ret = one_player_turn(hubState, currPlayer);
            if (ret != H_NORMAL) {
                return ret;
            }
        }
        print_round_cards(hubState); // print cards played this round
        gs_end_round(gameState); // finalises round and declare winner
    }
    // print final scores to terminal and return.
    print_player_scores(hubState);
    return H_NORMAL;
}

/* Runs the hub initialisation process, using the given hub and game state
 * structs to store state across the program.
 *
 * Performs argument checking, starts players, then offloads to other
 * functions.
 *
 * Sends GAMEOVER to all children before exiting in all cases.
 */
HubExitCode exec_hub_main(int argc, char** argv, HubState* hubState,
        GameState* gameState) {
    if (argc < 5) {
        return H_INCORRECT_ARGS;
    }
    int threshold = parse_int(argv[2]);
    if (threshold < 2) { // includes error case -1
        return H_INCORRECT_THRESHOLD;
    }
    int numPlayers = argc - 3; // 3 non-player arguments
    char** playerNames = argv + 3; // skip first 3 arguments



    Deck deck = {0}; // to store the full deck before dealing
    if (!deck_init_file(&deck, argv[1])) {
        return H_DECK_ERROR;
    }
    if (deck.numCards < numPlayers) {
        deck_destroy(&deck);
        return H_DECK_SHORT;
    }

    gs_init(gameState, numPlayers, threshold);
    hs_init(hubState, gameState);

    hs_deal_cards(hubState, &deck);
    deck_destroy(&deck); // we wont need this anymore

    noop_printf("hub PID: %d\n", getpid());
    // start child players and waits for their @ symbol
    for (int p = 0; p < numPlayers; p++) {
        if (!start_player(hubState, p, playerNames[p])) {
            return H_PLAYER_ERROR;
        }
    }
    // send players their hands.
    if (!send_player_hands(hubState)) {
        return H_PLAYER_EOF; // should we send GAMEOVER here?
    }
    // runs main game loop
    HubExitCode ret = exec_hub_loop(hubState);
    // sends GAMEOVER to everyone on, ignore return status.
    broadcast_message(hubState, msg_game_over(), -1);
    return ret;
}

/* Signal handler for SIGHUP.
 * Exits with the appropriate exit code and message and kills children.
 */
void sighup_handler(int signal) {
    // only if the game has been started
    if (hubStateGlobal->gameState != NULL) {
        for (int p = 0; p < hubStateGlobal->gameState->numPlayers; p++) {
            pid_t pid = hubStateGlobal->pids[p];
            // if this child is already dead (mo shindeiru) we don't kill it
            if (pid != 0) {
                // kill the child with this pid :o
                write(STDERR_FILENO, "killing", 7);
                kill(pid, SIGINT);
            }
        }
    }

    // not calling print_hub_message because that uses fprintf
    // it is possible write failes or writes too few bytes.
    write(STDERR_FILENO, "Ended due to signal\n", 20);
    _exit(H_SIGNAL); // exit immediately
}

void sigchld_handler(int signal, siginfo_t* siginfo, void* context) {
    write(STDERR_FILENO, "died", 4);
    if (hubStateGlobal->gameState == NULL) {
        return; // game not initialised yet.
    }
    write(STDERR_FILENO, "clearing", 8);
    pid_t pid = siginfo->si_pid;
    // find the player with that pid.
    for (int p = 0; p < hubStateGlobal->gameState->numPlayers; p++) {
            write(STDERR_FILENO, "found", 5);
        if (hubStateGlobal->pids[p] == pid) {
            // this player has died. we don't need to kill them on SIGHUP
            hubStateGlobal->pids[p] = (pid_t)0;
            break;
        }
    }
}

/* Registers the SIGHUP handler to exit the program.
 */
void register_sighup(void) {
    struct sigaction sa = new_sigaction();
    sa.sa_handler = sighup_handler;
    sigaction(SIGHUP, &sa, NULL);
}

void register_sigchld(void) {
    struct sigaction sa = new_sigaction();
    // don't be notified if a child stops temporarily, restart system calls
    // and use more detailed sigaction handler
    sa.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = sigchld_handler;
    sigaction(SIGCHLD, &sa, NULL);
}

/* Entry point of hub process. Sets signal handlers, manages initialisation
 * and destruction of states, and prints error messages. */
int main(int argc, char** argv) {
    GameState gameState = {0};
    HubState hubState = {0};
    hubStateGlobal = &hubState;

    // ignore SIGPIPE caused by writes to a dead child
    ignore_sigpipe();
    // register handlers to record and kill children.
    register_sigchld();
    register_sighup();


    HubExitCode ret = exec_hub_main(argc, argv, &hubState, &gameState);

    gs_destroy(&gameState);
    hs_destroy(&hubState);

    print_hub_message(ret);
    noop_printf("exiting hub with code: %d\n", ret);
    return ret;
}
