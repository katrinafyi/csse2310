#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include "deck.h"
#include "board.h"

#define NUM_PLAYERS 2
#define NUM_HAND 6

typedef struct GameState {
    int currPlayer;
    int numDrawn;
    Card playerHands[NUM_PLAYERS*NUM_HAND];
    char* deckFile;
    Deck* deck;

    BoardState* boardState;
} GameState;

bool load_game_file(GameState* gameState, char* saveFile);
void init_game_state(GameState* gameState);
bool save_game_file(GameState* gameState, char* saveFile);
bool deal_cards(GameState* gameState);
Card draw_card(GameState* gameState);
int main_loop(GameState* gameState, char* playerTypes);
void print_hand(GameState* gameState, int playerIndex);
void prompt_move();
void play_auto_turn();

#endif

