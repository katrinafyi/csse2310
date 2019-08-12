#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "deck.h"
#include "board.h"
#include "scoring.h"
#include "util.h"

/* Returns the card at the given position. Thin wrapper around get_board_cell.
 */
Card get_card_at(BoardState* bs, Position pos) {
    return *get_board_cell(bs, pos.r, pos.c);
}

// see header
int compute_longest_path(BoardState* boardState, char target, Position pos,
        int length) {
    /* the path restrictions in the spec impose a directed acyclic graph
     * structure on the board. this means the longest path algorithm is very
     * easy, we just need to check starting and ending suits. this is a
     * recursive DP implementation of longest path for DAG.
     */
    BoardState* bs = boardState;
    Card thisCard = get_card_at(bs, pos);
    // m is currently the length of the longest path to THIS card and will
    // be returned if there are no possible paths from this card.
    int m = 0; // start at 0, assuming no other paths and differing  suit.
    if (!is_null_card(thisCard) && thisCard.suit == target) {
        m = length; // this pos would be a valid endpoint.
    }
    // iterates over all 9 'shifts' from pos.
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dc * dc == dr * dr) { // for x = 0, 1, -1, x*x == abs(x)
                continue; // ignore 4 shifts on diagonals. TODO: probably slow
            }
            // DEBUG_PRINTF("testing shift dr %d, dc %d\n", dr, dc);
            Position newPos = pos;
            newPos.r = mod(newPos.r + dr, bs->height); // wrap around board
            newPos.c = mod(newPos.c + dc, bs->width);
            Card newCard = get_card_at(bs, newPos);
            // don't move to null cards or cards <= this card.
            if (is_null_card(newCard) || newCard.num <= thisCard.num) {
                continue;
            }
            int l = compute_longest_path(bs, target, newPos, length + 1);
            m = (l > m) ? l : m; // m = max(m, l)
        }
    }
    return m;
}

// iterates over every starting position and looks for longest path from
// that position to any card of the same suit.
void longest_letter_paths(BoardState* boardState, int* letterLengths) {
    // because we use letterLengths in comparisons, zero it first.
    memset(letterLengths, 0, sizeof(int) * NUM_LETTERS);
    BoardState* bs = boardState;
    int w = bs->width;
    int h = bs->height;
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            Card card = *get_board_cell(bs, r, c);
            if (is_null_card(card)) {
                continue;
            }
            int len = compute_longest_path(bs, card.suit,
                    (Position) {r, c}, 0);
            // assumes card.suit is always valid.
            int letter = card.suit - 'A';
            DEBUG_PRINTF("%d length from (%d,%d) : %d%c\n",
                    len, r, c, card.num, card.suit);
            if (len > letterLengths[letter]) {
                letterLengths[letter] = len;
            }
        }
    }
    // result stored into letterLengths array.
}
