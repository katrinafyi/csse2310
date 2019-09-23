#ifndef HUBSTATE_H
#define HUBSTATE_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include "gameState.h"
#include "deck.h"

/* Holds two ends of a pipe corresponding to the read/write ends (relative to
 * the hub) for a particular player.
 */
typedef struct PipePair {
    FILE* read;
    FILE* write;
} PipePair;

/* Entire struct for the hub. Contains gameState struct as well as pipes for
 * communication and copies of all player's hands.
 */
typedef struct HubState {
    GameState* gameState;

    // array of pipes for each player.
    PipePair* pipes;
    // array of hands for each player.
    Deck* playerHands;
    // array if player PIDs for SIGHUP handler.
    pid_t* pids;
} HubState;

/* Attachs the given gameState to the hubState. Allocates memory for arrays
 * based on numPlayers in gameState.
 *
 * Should be called after gameState is initialised!
 */
void hs_init(HubState* hubState, GameState* gameState);

/* Frees memory associated with this state struct and all contained structs.
 */
void hs_destroy(HubState* hubState);


/* Deals cards to players from the given deck. Uses state from hubState's
 * gameState and stores in playerHands.
 */
void hs_deal_cards(HubState* hubState, Deck* deck);

/* Adds the player with the given index and PID.
 * Sets the pipes for the given player to the given file pointers, storing
 * them in hubState's fields.
 */
void hs_add_player(HubState* hubState, int player, pid_t pid, FILE* readFile,
        FILE* writeFile);

/* Removes the given card from the given player's hand.
 */
void hs_card_played(HubState* hubState, int player, Card card);

#endif
