#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "deck.h"
#include "board.h"
#include "util.h"

bool is_on_board(BoardState* boardState, int r, int c) {
    return (0 <= r && r < boardState->height)
        && (0 <= c && c < boardState->width);

}

void init_board(BoardState* boardState, int width, int height) {
    boardState->board = malloc(sizeof(Card)*width*height);
    boardState->width = width;
    boardState->height = height;

    for (int i = 0; i < width*height; i++) {
        // printf("initialising %d to null\n", i);
        boardState->board[i] = NULL_CARD;
    }
}

bool has_card_at(BoardState* boardState, int row, int col) {
    return boardState->board[row*boardState->width + col].num != 0;
}

// WARNING: lazy implementation. for negatives, only valid up to -d.
int mod(int x, int d) {
    if (x < 0) {
        return x + d;
    } else {
        return x % d;
    }
}

bool has_adjacent(BoardState* boardState, int row, int col) {
    if (!is_on_board(boardState, row, col)) {
        return false; // surely not if card is not even on board.
    }
    int w = boardState->width;
    int h = boardState->height;
    // because board wraps around, we use mod.
    return has_card_at(boardState, mod(row-1, h), mod(col, w))
        || has_card_at(boardState, mod(row+1, h), mod(col, w))
        || has_card_at(boardState, mod(row, h), mod(col-1, w))
        || has_card_at(boardState, mod(row, h), mod(col+1, w));
}

bool place_card(BoardState* boardState, int row, int col, Card card) {
    int w = boardState->width;
    if (!is_on_board(boardState, row, col)
            || has_card_at(boardState, row, col)) {
        return false;
    }
    if (!has_adjacent(boardState, row, col) && !is_board_empty(boardState)) {
        // has no adjacent cards and board is not empty.
        return false;
    }
    boardState->board[row*w + col] = card;
    return true;
}

void print_board(BoardState* boardState) {
    int w = boardState->width;
    int h = boardState->height;
    for (int i = 0; i < w*h; i++) {
        char str[3];
        fmt_card(str, boardState->board[i]);
        printf("%s", str);
        if ((i+1) % w == 0) {
            printf("\n");
        }
    }
}

bool is_board_full(BoardState* boardState) {
    for (int i = 0; i < boardState->width * boardState->height; i++) {
        if (is_null_card(boardState->board[i])) {
            return false;
        }
    }
    return true;
}

bool is_board_empty(BoardState* boardState) {
    int w = boardState->width;
    int h = boardState->height;
    for (int i = 0; i < w*h; i++) {
        if (!is_null_card(boardState->board[i])) {
            return false;
        }
    }
    return true;
}

bool is_size_valid(int width, int height) {
    return MIN_SIZE <= width && width <= MAX_SIZE
        && MIN_SIZE <= height && height <= MAX_SIZE;
}
