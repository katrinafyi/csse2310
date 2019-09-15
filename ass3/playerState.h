#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H

#include "deck.h"
#include "gameState.h"

typedef struct {
    GameState* gameState;

    int playerIndex;
    int handSize;
    Deck* hand;
} PlayerState;

void ps_init(PlayerState* playerState, GameState* gameState, int playerIndex);
void ps_destroy(PlayerState* playerState);

void ps_set_hand(PlayerState* playerState, Deck* hand);
void ps_play(PlayerState* playerState, Card card);

#endif
