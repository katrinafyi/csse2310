#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "hubState.h"
#include "util.h"

// see header
void hs_init(HubState* hubState, GameState* gameState) {
    hubState->playerHands = NULL; // not initialised until cards dealt.
    hubState->pipes = calloc(gameState->numPlayers, sizeof(PipePair));

    hubState->gameState = gameState;
}

// see header
void hs_destroy(HubState* hubState) {
    GameState* gameState = hubState->gameState;

    if (hubState->playerHands != NULL) {
        for (int i = 0; i < gameState->numPlayers; i++) {
            deck_destroy(hubState->playerHands + i);
        }
        free(hubState->playerHands);
        hubState->playerHands = NULL;
    }
    if (hubState->pipes != NULL) {
        for (int i = 0; i < gameState->numPlayers; i++) {
            if (hubState->pipes[i].read != NULL) {
                fclose(hubState->pipes[i].read);
            }
            if (hubState->pipes[i].write != NULL) {
                fclose(hubState->pipes[i].write);
            }
            hubState->pipes[i] = (PipePair) { NULL, NULL };
        }
        free(hubState->pipes);
        hubState->pipes = NULL;
    }
    if (hubState->gameState != NULL) {
        gs_destroy(hubState->gameState);
        hubState->gameState = NULL;
    }
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
            Card card = deck->cards[drawn];
            DEBUG_PRINTF("dealing %c%x to %d\n", card.suit, card.rank, p);
            hubState->playerHands[p].cards[i] = card;
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
    deck_remove_card(hubState->playerHands + player, card);
}
