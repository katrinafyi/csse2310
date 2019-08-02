#ifndef LONGESTPATH_H
#define LONGESTPATH_H

#include "board.h"

#define NUM_LETTERS 26

typedef struct Position {
    int r;
    int c;
} Position;

int compute_longest_path(BoardState* boardState, char target, Position pos,
        int length);
void longest_letter_paths(BoardState* boardState, int* letterLengths);

#endif
