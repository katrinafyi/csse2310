#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdio.h>

#include "deck.h"

#define MIN_SIZE 3
#define MAX_SIZE 100

typedef struct BoardState {
    int width;
    int height;
    Card* board;
} BoardState;

void init_board(BoardState* boardState, int width, int height);
Card* get_card_ptr(BoardState* boardState, int row, int col);
bool place_card(BoardState* boardState, int row, int col, Card card);
void print_board(BoardState* boardState);
bool is_on_board(BoardState* boardState, int r, int c);
bool is_board_full(BoardState* boardState);
bool is_board_empty(BoardState* boardState);
bool is_size_valid(int width, int height);
int mod(int x, int d);

#endif

