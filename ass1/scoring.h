#ifndef LONGESTPATH_H
#define LONGESTPATH_H

#include "board.h"

// will this ever change
#define NUM_LETTERS 26

/* Simple tuple of row and column on the board. */
typedef struct Position {
    int r;
    int c;
} Position;

/* Returns the length of the longest path starting at the given pos
 * and ending at a card of the target suit.
 *
 * Moves can only be made from cards to cards of strictly greater rank.
 * Moves can wrap around board edges, same as adjacency for card placement.
 *
 * Length is the length to pos, used for recursion. When calling the base
 * case, this should be 0.
 */
int compute_longest_path(BoardState* boardState, char target, Position pos,
        int length);
/* Computes the longest paths for each letter, storing it in the given
 * letterLengths array. letterLengths should contain space for NUM_LETTERS
 * integers.
 *
 * 'Length' is defined as the number of edges in the path. That is,
 * number of cards - 1.
 * Looks for paths starting from any position and ending at any card of the
 * same suit, subject to path conditions in compute_longest_path().
 */
void longest_letter_paths(BoardState* boardState, int* letterLengths);

#endif
