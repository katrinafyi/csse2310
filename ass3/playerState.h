#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H

#include "deck.h"
#include "gameState.h"

/* Struct for all the player's state, containing a gameState.
 * In addition, stores this player's index, the hand size and their current
 * hand.
 */
typedef struct PlayerState {
    GameState* gameState;

    int playerIndex;
    int handSize;
    Deck* hand; // malloc'd, but should not be manually free'd
} PlayerState;

/* Initialises a new playerState struct, attaching the given gameState struct
 * and with the given player index.
 */
void ps_init(PlayerState* playerState, GameState* gameState, int playerIndex);

/* Destroys this playerState struct and any contained structs, freeing memory
 */
void ps_destroy(PlayerState* playerState);

/* Sets the player's hand to the given hand. The given Deck struct is COPIED
 * into the player's hand struct.
 */
void ps_set_hand(PlayerState* playerState, Deck* hand);
/* The current player has played the given card. Removes it from the hand.
 */
void ps_play(PlayerState* playerState, Card card);

#endif
