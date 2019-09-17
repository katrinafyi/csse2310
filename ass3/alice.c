#include <stdio.h>
#include <stdlib.h>

#include "deck.h"
#include "strategy.h"
#include "util.h"

/* Returns a card index to play from the given player state's hand for Alice's
 * strategy when leading.
 * Chooses highest card of SCDH, in that order.
 */
int strategy_when_leading(PlayerState* playerState) {
    return deck_search(playerState->hand, "SCDH", true);
}

/* Returns a card index to play from the given player state hand for Alice
 * when following.
 * Chooses lowest of lead suit or highest of DHSC in that order if we have no
 * lead suit cards.
 */
int strategy_when_following(PlayerState* playerState) {
    char leadSuit = playerState->gameState->leadSuit;

    int leadIndex = deck_best_card(playerState->hand, leadSuit, false);
    if (leadIndex != -1) {
        return leadIndex;
    }

    return deck_search(playerState->hand, "DHSC", true);
}

