#include <stdlib.h>
#include <stdio.h>

#include "deck.h"
#include "strategy.h"
#include "util.h"

int strategy_when_leading(PlayerState* playerState) {
    DEBUG_PRINT("bob leading");
    return deck_search(playerState->hand, "DHSC", false);
}

int strategy_when_following(PlayerState* playerState) {
    int numPlayers = playerState->gameState->numPlayers;
    int threshold = playerState->gameState->threshold;
    char leadSuit = playerState->gameState->leadSuit;

    bool condition1 = false; // naming things is v hard
    for (int p = 0; p < numPlayers; p++) {
        // someone has won at least thresohld-2 cards
        if (playerState->gameState->diamondsWon[p] >= threshold - 2) {
            condition1 = true;
            break;
        }
    }
    // some D cards have been played this round
    Deck* table = playerState->gameState->table;
    bool condition2 = (deck_best_card(table, 'D', true) != -1);
    
    // alternate strategy if both diamond conditions met
    bool altStrategy = condition1 && condition2;
    DEBUG_PRINTF("bob conditions: %d %d -> %d\n", condition1, condition2,
            altStrategy);

    // highest if alt strat, else lowest
    int leadIndex = deck_best_card(playerState->hand, leadSuit, altStrategy);
    if (leadIndex != -1) {
        return leadIndex;
    }

    // order depends on strategy, play lowest if alt else highest.
    char* order = altStrategy ? "SCHD" : "SCDH";
    return deck_search(playerState->hand, order, !altStrategy);
}

