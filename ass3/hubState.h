#ifndef HUBSTATE_H
#define HUBSTATE_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "gameState.h"
#include "deck.h"

typedef struct PipePair {
  FILE* read;
  FILE* write;
} PipePair;

typedef struct HubState {
  GameState* gameState;

  PipePair* pipes;
  Deck* playerHands;
} HubState;

// only mallocs everything.
void hs_init(HubState* hubState, GameState* gameState);

void hs_destroy(HubState* hubState);


// get numPlayers from gs and deal cards.
void hs_deal_cards(HubState* hubState, Deck* deck);
void hs_set_player_pipe(HubState* hubState, int player, FILE* readFile, FILE* writeFile);

// removes card from their hand.
void hs_played_card(HubState* hubState, int player, Card card);

#endif
