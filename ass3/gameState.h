#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdbool.h>

#include "deck.h"

/* Struct to hold all common game state, shared between players and hub.
 * The functions here are called by both player and hub to keep the state in
 * sync.
 */
typedef struct GameState {
    // number of player in the game
    int numPlayers;
    // threshold for D to count positive
    int threshold;

    // lead player this round
    int leadPlayer;
    // lead suit this round
    char leadSuit;
    // current player
    int currPlayer;

    // cards played this round, indexed by player ID
    Deck* table;
    // diamonds won for each player
    int* diamondsWon;
    // points won for each player
    int* playerPoints;
} GameState;

/* Initialises the game state with the given number of players and threshold.
 */
void gs_init(GameState* gameState, int numPlayers, int threshold);

/* Cleans up memory associated with the struct and any sub-structs.
 *
 * Designed to be idempotent on multiple calls and can be safely called on a
 * zero-initialised struct.
 */
void gs_destroy(GameState* gameState);

/* Sets the state for the start of a new round, led by the given player.
 * Sets currPlayer to the leader.
 */
void gs_new_round(GameState* gameState, int leadPlayer);

/* A card has been played by some player. Player and card are as given.
 * Increments currPlayer to next player in the rotation after this player.
 */
void gs_card_played(GameState* gameState, int player, Card card);

/* Ends the round, determining the winner and incrementing diamondsWon and
 * playerPoints as appropriate. Also sets the winner to the lead player.
 */
void gs_end_round(GameState* gameState);

#endif
