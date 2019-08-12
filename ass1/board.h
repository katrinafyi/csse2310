#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdio.h>

#include "deck.h"

#define MIN_SIZE 3
#define MAX_SIZE 100

/* Current state of the board, storing dimensions and an array of cards.
 * Flattens the 2D board, storing it as a 1D array. However, accesses
 * to the board should be made via get_board_cell().
 */
typedef struct BoardState {
    int width;
    int height;
    int numPlaced;
    Card* board;
} BoardState;

/* Initialises the board state with the given width and height.
 * MALLOCs enough space for width*height cards.
 */
void init_board(BoardState* boardState, int width, int height);

/* Recounts the number of cards currently placed on the board,
 * storing it into the numPlaced member. */
void count_cards(BoardState* boardState);

/* Returns a pointer to the card at the given row/col.
 * Adding to the returned pointer moves right along the current row
 * then wrapping around to the next row.
 */
Card* get_board_cell(BoardState* boardState, int row, int col);

/* Places the given card in the given row/col.
 * Returns false if card cannot be placed there.
 * A card can be placed at a given position if:
 *  - board is empty; or
 *  - there exists a card directly adjacent to this position,
 *    where the board's edges wrap around.
 */
bool place_card(BoardState* boardState, int row, int col, Card card);

/* Prints the board's state to stdout. Blank cards are filled with
 * BLANK_CHAR_PRINT.
 */
void print_board(BoardState* boardState);

/* Prints the board to the file given by file. Blank cards are filled in with
 * the given blank character.
 * Returns true on success, false on failure.
 */
bool fprint_board(BoardState* boardState, FILE* file, char blank);

/* Returns true if the given row/col is a valid position for the
 * board.
 */
bool is_on_board(BoardState* boardState, int r, int c);

/* Returns true if the board is full of cards. That is, there is no
 * space for any more cards.
 */
bool is_board_full(BoardState* boardState);

/* Returns true if the board contains no cards. */
bool is_board_empty(BoardState* boardState);

/* Returns true if the given width/height are within the bounds specified
 * by the spec. That is, both dimensions must be >= MIN_SIZE and <= MAX_SIZE.
 */
bool is_size_valid(int width, int height);

/* Returns x (mod d). */
int mod(int x, int d);

#endif

