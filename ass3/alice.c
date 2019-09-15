#include <stdio.h>
#include <stdlib.h>

#include "deck.h"
#include "strategy.h"
#include "util.h"

int strategy_when_leading(PlayerState* playerState) {
    return deck_search(playerState->hand, "SCDH", true);
}

int strategy_when_following(PlayerState* playerState) {
    char leadSuit = playerState->gameState->leadSuit;

    int leadIndex = deck_best_card(playerState->hand, leadSuit, false);
    if (leadIndex != -1) {
        return leadIndex;
    }

    return deck_search(playerState->hand, "DHSC", true);
}

