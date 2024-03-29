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
    // width of the board
    int width;
    // height of the board
    int height;
    // number of cards placed on the board
    int numPlaced;
    // malloc'd array storing the board's cards
    Card* board;
    // malloc'd buffer for printing the board.
    char* printBuffer;
} BoardState;

/* Initialises the board state with the given width and height.
 * MALLOCs enough space for width*height cards.
 */
void init_board(BoardState* boardState, int width, int height);

/* Returns a new board state with values initialised to NULLs.
 */
BoardState new_board(void);

/* Frees all allocated memory from the given board struct.
 */
void destroy_board(BoardState* boardState);

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

/* Returns x (mod d).
 *
 * WARNING: lazy implementation. for negatives, only valid up to -d.
 * This is fine for the uses here because we only use it to wrap around.
 */
int mod(int x, int d);


#endif

