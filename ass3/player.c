#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "exitCodes.h"
#include "strategy.h"
#include "util.h"
#include "gameState.h"
#include "playerState.h"
#include "messages.h"

/* Returns true if the player should exit upon receiving the given message
 * with given status. If true, stores the appropriate exit code in outCode.
 * If false, does not change outCode.
 * NULL can be passed to message to skip checking for MSG_GAME_OVER.
 *
 * This only returns true if exiting is correct behaviour in all cases. This
 * does not verify the message type is valid (contextually) for a particular
 * caller.
 */
bool player_should_exit(MessageStatus status, Message* message,
        PlayerExitCode* outCode) {
    if (status != MS_OK) {
        DEBUG_PRINTF("error message status: %d\n", status);
        *outCode = status == MS_EOF ? P_HUB_EOF : P_INVALID_MESSAGE;
        return true;
    }
    if (message != NULL && message->type == MSG_GAME_OVER) {
        DEBUG_PRINT("flagging exit due to GAMEOVER");
        *outCode = P_NORMAL;
        return true;
    }
    return false;
}

Card get_card_to_play(PlayerState* playerState) {
    GameState* gameState = playerState->gameState;
    int playerNum = playerState->playerIndex;

    assert(!deck_is_empty(playerState->hand)); // sanity check
    int cardIndex;
    if (gameState->leadPlayer == playerNum) {
        cardIndex = strategy_when_leading(playerState);
    } else {
        cardIndex = strategy_when_following(playerState);
    }
    return playerState->hand->cards[cardIndex];
}

/* Plays a single round of the game. Assumes the lead player is already set.
 * Returns an exit code. If the outContinue parameter is set to true,
 * play should continue to the next round if exit code is normal, otherwise
 * it should exit.
 */
PlayerExitCode play_round(PlayerState* playerState, bool* outContinue) {
    PlayerExitCode ret = P_INVALID_MESSAGE;
    MessageStatus status;
    Message message;
    *outContinue = false; // by default, don't continue.

    GameState* gameState = playerState->gameState;
    int numPlayers = gameState->numPlayers;
    int playerNum = playerState->playerIndex;
    for (int i = 0; i < numPlayers; i++) {
        int currPlayer = gameState->currPlayer;
        DEBUG_PRINTF("player turn: %d\n", currPlayer);
        if (currPlayer == playerNum) {
            DEBUG_PRINT("playing our turn"); // it's our turn

            Card card = get_card_to_play(playerState);
            status = msg_send(stdout, msg_play_card(card));
            if (player_should_exit(status, NULL, &ret)) {
                return ret;
            }

            ps_play(playerState, card);
            gs_play_turn(gameState, currPlayer, card);
        } else { // it's someone else's turn
            DEBUG_PRINT("other turn, waiting for message");
            status = msg_receive(stdin, &message);

            if (player_should_exit(status, &message, &ret) ||
                    message.type != MSG_PLAYED_CARD) {
                return ret;
            }

            PlayedTuple played = message.data.playedTuple;
            gs_play_turn(gameState, played.player, played.card);
        }
    }
    // at this point, everyone's taken a turn and we should go to next round
    *outContinue = true;
    return P_NORMAL;
}

PlayerExitCode exec_player_loop(PlayerState* playerState) {
    Message message;
    PlayerExitCode ret;

    DEBUG_PRINT("expecting hand");
    MessageStatus status = msg_receive(stdin, &message);
    if (player_should_exit(status, &message, &ret) ||
            message.type != MSG_HAND) {
        return ret;
    }
    Deck hand = message.data.hand; // temporarily copy hand
    ps_set_hand(playerState, &hand); // ps_set_hand copies hand.
    if (hand.numCards != playerState->handSize) { // verify hand size
        DEBUG_PRINT("hand size doesn't match argument");
        return P_INVALID_MESSAGE;
    }

    GameState* gameState = playerState->gameState;
    while (true) {
        DEBUG_PRINT("expecting new round");
        status = msg_receive(stdin, &message);
        if (player_should_exit(status, &message, &ret) ||
                message.type != MSG_NEW_ROUND) {
            return ret;
        }
        gs_new_round(gameState, message.data.leadPlayer);

        // flag is needed to allow exits when ret is P_NORMAL. this occurs
        // at game over.
        bool cont = false; 
        ret = play_round(playerState, &cont);
        if (ret != P_NORMAL || !cont) {
            return ret;
        }
        gs_end_round(gameState);
    }
    assert(0); // will always return from within loop
}

PlayerExitCode exec_player_main(int argc, char** argv, GameState* gameState,
        PlayerState* playerState) {
    // args: player numPlayers playerNum threshold handSize
    if (argc != 5) {
        return P_INCORRECT_ARGS;
    }

    int numPlayers = parse_int(argv[1]);
    if (numPlayers < 2) {
        return P_INCORRECT_PLAYERS;
    }

    int playerNum = parse_int(argv[2]);
    if (playerNum < 0 || playerNum >= numPlayers) {
        return P_INCORRECT_POSITION;
    }

    int threshold = parse_int(argv[3]);
    if (threshold < 2) {
        return P_INCORRECT_THRESHOLD;
    }

    int handSize = parse_int(argv[4]);
    if (handSize < 1) {
        return P_INCORRECT_HAND;
    }
    playerState->handSize = handSize;

    gs_init(gameState, numPlayers, threshold);
    ps_init(playerState, gameState, playerNum);

    printf("@");
    fflush(stdout);
    return exec_player_loop(playerState);
}

int main(int argc, char** argv) {
    ignore_sigpipe();

    PlayerState playerState = {0};
    GameState gameState = {0};

    PlayerExitCode ret = exec_player_main(argc, argv, 
            &gameState, &playerState);
    
    ps_destroy(&playerState);
    gs_destroy(&gameState);

    print_player_message(ret);
    DEBUG_PRINTF("player exiting with code: %d\n", ret);
    return ret;
}