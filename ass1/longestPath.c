#include <stdlib.h>
#include <stdio.h>

#include "longestPath.h"
#include "util.h"

Card get_card_at(BoardState* bs, Position pos) {
    return bs->board[pos.r*bs->width + pos.c];
}

int compute_longest_path(BoardState* boardState, char target, Position pos,
        int length) {
    BoardState* bs = boardState;
    Card thisCard = get_card_at(bs, pos);
    int m = 0; // 0 unless this card is valid endpoint or path exists.
    if (thisCard.num != 0 && thisCard.suit == target) {
        m = length; // this pos would be a valid endpoint.
    }
    // iterates over all 9 'shifts' from pos.
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (abs(dc) == abs(dr)) {
                continue; // ignore shifts on diagonals
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
            int l = compute_longest_path(bs, target, newPos, length+1);
            // DEBUG_PRINTF("len %d\n", l);
            m = (l > m) ? l : m;
        }
    }
    return m;
}
