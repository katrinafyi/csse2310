#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

#include "deck.h"

typedef struct BoardState {
    int width;
    int height;
    Card* board;
} BoardState;

void init_board(BoardState* boardState, int width, int height);
bool place_card(BoardState* boardState, int row, int col, Card card);
void print_board(BoardState* boardState);
int compute_longest_path(BoardState* boardState, char suit);

#endif

