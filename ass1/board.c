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
    boardState->board = malloc(sizeof(Card) * width * height);
    boardState->width = width;
    boardState->height = height;
    boardState->numPlaced = 0;

    for (int i = 0; i < width * height; i++) {
        // printf("initialising %d to null\n", i);
        boardState->board[i] = NULL_CARD;
    }
}

void count_cards(BoardState* boardState) {
    int count = 0;
    for (int r = 0; r < boardState->height; r++) {
        for (int c = 0; c < boardState->width; c++) {
            if (has_card_at(boardState, r, c)) {
                count++:
            }
        }
    }
    boardState->numPlaced = count;
}

Card* get_board_cell(BoardState* boardState, int row, int col) {
    assert(is_on_board(boardState, row, col));
    return boardState->board + row * boardState->width + col; // get_board_cell
}

bool has_card_at(BoardState* boardState, int row, int col) {
    assert(is_on_board(boardState, row, col));
    return !is_null_card(*get_board_cell(boardState, row, col));
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
    assert(is_on_board(boardState, row, col));
    int w = boardState->width;
    int h = boardState->height;
    // because board wraps around, we use mod.
    return has_card_at(boardState, mod(row - 1, h), mod(col, w))
            || has_card_at(boardState, mod(row + 1, h), mod(col, w))
            || has_card_at(boardState, mod(row, h), mod(col - 1, w))
            || has_card_at(boardState, mod(row, h), mod(col + 1, w));
}

bool place_card(BoardState* boardState, int row, int col, Card card) {
    assert(is_on_board(boardState, row, col));
    // if already card at this pos, fail.
    if (has_card_at(boardState, row, col)) {
        return false;
    }
    // require either adjacent card or board is empty.
    if (!has_adjacent(boardState, row, col) && !is_board_empty(boardState)) {
        return false;
    }
    *get_board_cell(boardState, row, col) = card;
    boardState->numPlaced++;
    return true;
}

void print_board(BoardState* boardState) {
    // TODO: accept FILE* and blank params for print_board
    int w = boardState->width;
    int h = boardState->height;
    for (int i = 0; i < w * h; i++) {
        char str[3];
        printf("%s", fmt_card(str, boardState->board[i]));
        if ((i + 1) % w == 0) {
            printf("\n");
        }
    }
}

bool is_board_full(BoardState* boardState) {
    // efficient checking using number of cards placed
    return boardState->numPlaced == boardState->width * boardState->height;
}

bool is_board_empty(BoardState* boardState) {
    return boardState->numPlaced == 0;
}

bool is_size_valid(int width, int height) {
    return MIN_SIZE <= width && width <= MAX_SIZE &&
            MIN_SIZE <= height && height <= MAX_SIZE;
}

