#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "hubState.h"

// see header
void hs_init(HubState* hubState, GameState* gameState) {
    hubState->playerHands = NULL; // not initialised until cards dealt.
    hubState->pipes = calloc(gameState->numPlayers, sizeof(PipePair));

    hubState->gameState = gameState;
}

// see header
void hs_destroy(HubState* hubState) {
    if (hubState->playerHands != NULL) {
        for (int i = 0; i < gameState->numPlayers; i++) {
            deck_destroy(hubState->playerHands + i);
        }
        free(hubState->playerHands);
        hubState->playerHands = NULL;
    }
    free(hubState->pipes);
    hubState->pipes = NULL;

    gs_destroy(hubState->gameState);
}

// see header
void hs_deal_cards(HubState* hubState, Deck* deck) {
    int numPlayers = hubState->gameState->numPlayers;

    hubState->playerHands = calloc(numPlayers, sizeof(Deck));

    int handSize = deck->numCards / numPlayers; // int division
    assert(handSize > 0);
    int drawn = 0;
    for (int p = 0; p < numPlayers; p++) {
        deck_init_empty(hubState->playerHands + p, handSize);
        for (int i = 0; i < handSize; i++) {
            hubState->playerHands[p].cards[i] = deck->cards[drawn];
            drawn++;
        }
    }
    assert(drawn == handSize * numPlayers);
}

// see header
void hs_set_player_pipe(HubState* hubState, int player,
        FILE* readFile, FILE* writeFile) {
    hubState->pipes[player] = (PipePair) { .read = readFile,
            .write = writeFile };
}

// see header
void hs_played_card(HubState* hubState, int player, Card card) {
    int index = deck_index_of(hubState->playerHands + player);
    assert(index >= 0);
    hubState->playerHands[player].cards[index] = NULL_CARD;
}
