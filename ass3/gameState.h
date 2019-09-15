#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdbool.h>

#include "deck.h"

typedef struct {
    int numPlayers;
    int threshold;

    int roundNum;
    int leadPlayer;
    char leadSuit;
    int currPlayer;


    Deck* table;
    int* diamondsWon;
    int* playerPoints;
} GameState;

void gs_init(GameState* gameState, int numPlayers, int threshold);
void gs_destroy(GameState* gameState);

void gs_new_round(GameState* gameState, int leadPlayer);

void gs_place_card(GameState* gameState, int player, Card card);
bool gs_is_round_over(GameState* gameState);

void gs_end_round(GameState* gameState);

#endif
