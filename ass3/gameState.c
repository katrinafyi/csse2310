#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gameState.h"
#include "deck.h"
#include "util.h"

// see header
void gs_init(GameState* gameState, int numPlayers, int threshold) {
    gameState->numPlayers = numPlayers;
    gameState->threshold = threshold;

    Deck* table = malloc(sizeof(Deck));
    deck_init_empty(table, numPlayers);
    gameState->table = table;

    // calloc initialises memory to 0
    gameState->diamondsWon = calloc(numPlayers, sizeof(int));
    gameState->playerPoints = calloc(numPlayers, sizeof(int));
}

// see header
void gs_destroy(GameState* gameState) {
    if (gameState->table != NULL) {
        deck_destroy(gameState->table);
        free(gameState->table);
        gameState->table = NULL;
    }
    if (gameState->diamondsWon != NULL) {
        free(gameState->diamondsWon);
        gameState->diamondsWon = NULL;
    }
    if (gameState->playerPoints != NULL) {
        free(gameState->playerPoints);
        gameState->playerPoints = NULL;
    }
}

void gs_new_round(GameState* gameState, int leadPlayer) {
    DEBUG_PRINT("new round");
    gameState->leadPlayer = leadPlayer;
    gameState->currPlayer = leadPlayer;
    deck_clear(gameState->table);
}

void gs_place_card(GameState* gameState, int player, Card card) {
    DEBUG_PRINT("placing card");
    assert(player == gameState->currPlayer);

    if (player == gameState->leadPlayer) {
        DEBUG_PRINTF("setting lead suit %c\n", card.suit);
        gameState->leadSuit = card.suit;
    }
    assert(0 <= player && player < gameState->table->numCards);
    gameState->table->cards[player] = card;

    gameState->currPlayer++;
    gameState->currPlayer %= gameState->numPlayers;
}

bool gs_is_round_over(GameState* gameState) {
    // every player has made a turn and the table is full.
    return deck_is_full(gameState->table);
}

void gs_end_round(GameState* gameState) {
    DEBUG_PRINT("ending round");

    int winningPlayer = deck_best_card_index(gameState->table,
            gameState->leadSuit, true);
    assert(winningPlayer >= 0);
    Card winningCard = gameState->table->cards[winningPlayer];
    DEBUG_PRINTF("player %d won with card %c%x\n", winningPlayer,
            winningCard.suit, winningCard.rank);
    assert(winningCard.suit == gameState->leadSuit);
    
    // count diamonds on the table.
    Deck* table = gameState->table;
    int diamonds = 0;
    for (int i = 0; i < table->numCards; i++) {
        if (table->cards[i].suit == 'D') {
            diamonds++;
        }
    }
    // increment points and give diamonds to winning player.
    gameState->playerPoints[winningPlayer]++;
    gameState->diamondsWon[winningPlayer] += diamonds;
}