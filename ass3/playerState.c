#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "playerState.h"

// see header
void ps_init(PlayerState* playerState, GameState* gameState, int playerIndex) {
    playerState->playerIndex = playerIndex;
    playerState->gameState = gameState;
    // calloc zeros memory
    playerState->hand = calloc(1, sizeof(Deck));
}

// see header
void ps_destroy(PlayerState* playerState) {
    if (playerState->gameState != NULL) {
        gs_destroy(playerState->gameState);
        playerState->gameState = NULL;
    }
    if (playerState->hand != NULL) {
        deck_destroy(playerState->hand);
        free(playerState->hand);
        playerState->hand = NULL;
    }
}

// see header
void ps_set_hand(PlayerState* playerState, Deck* hand) {
    // copy hand into a place we manage so its lifetime is the same as
    // playerState's lifetime.
    *(playerState->hand) = *hand;
}

// see header
void ps_play(PlayerState* playerState, Card card) {
    assert(playerState->playerIndex == playerState->gameState->currPlayer);
    deck_remove_card(playerState->hand, card);
}
