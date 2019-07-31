#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "deck.h"
#include "board.h"

void assert_on_board(BoardState* boardState, int r, int c) {
    assert(0 <= r && r < boardState->height);
    assert(0 <= c && c < boardState->width); 
}

void init_board(BoardState* boardState, int width, int height) {
    boardState->board = malloc(sizeof(Card)*width*height);
    boardState->width = width;
    boardState->height = height;
    
    for (int i = 0; i < width*height; i++) {
        boardState->board[i] = (Card) { 0, '!' };
    } 
}

int load_board_row(BoardState* boardState, int row, char* rowText) {
    // a
}

int place_card(BoardState* boardState, int row, int col, Card card) {
    int w = boardState->width;
    assert_on_board(boardState, row, col);
    boardState->board[row*w + col] = card;
}   

void print_board(BoardState* boardState) {
    int w = boardState->width;
    int h = boardState->height;
    for (int i = 0; i < w*h; i++) {
        char* cardStr = fmt_card(boardState->board[i]);
        printf("%s", cardStr);
        free(cardStr);
        if ((i+1) % w == 0) {
            printf("\n");
        }
    }
}
