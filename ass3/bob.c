#include <stdlib.h>
#include <stdio.h>

#include "deck.h"
#include "strategy.h"
#include "util.h"

/* Returns a card index to play from the given player state's hand for Alice's
 * strategy when leading.
 * Chooses highest card of DHSC, in that order.
 */
int strategy_when_leading(PlayerState* playerState) {
    noop_print("bob leading");
    return deck_search(playerState->hand, "DHSC", false);
}

/* Returns a card index to play from the given player state hand for Alice
 * when following.
 *
 * Strategy as defined in spec. Has alternate behaviour when any player has
 * won at least threshold - 2 D cards and at least 1 D card has been played
 * this round.
 */
int strategy_when_following(PlayerState* playerState) {
    int numPlayers = playerState->gameState->numPlayers;
    int threshold = playerState->gameState->threshold;
    char leadSuit = playerState->gameState->leadSuit;

    // if someone has won at least thresohld-2 D cards
    bool thresholdMet = false;
    for (int p = 0; p < numPlayers; p++) {
        if (playerState->gameState->diamondsWon[p] >= threshold - 2) {
            thresholdMet = true;
            break;
        }
    }
    Deck* table = playerState->gameState->table;
    // if some D cards have been played this round
    bool diamondPlayed = (deck_best_card(table, 'D', true) != -1);

    // alternate strategy if both diamond conditions met
    bool altStrategy = thresholdMet && diamondPlayed;
    noop_printf("bob conditions: %d %d -> %d\n", thresholdMet, diamondPlayed,
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

