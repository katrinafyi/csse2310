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
    gameState->currPlayer = 0;
    gameState->leadPlayer = 0;

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

// see header
void gs_new_round(GameState* gameState, int leadPlayer) {
    DEBUG_PRINTF("new round! led by %d\n", leadPlayer);
    // round number is set by gs_end_round
    gameState->leadPlayer = leadPlayer;
    gameState->currPlayer = leadPlayer;
    deck_clear(gameState->table);
}

// see header
void gs_card_played(GameState* gameState, int player, Card card) {
    assert(player == gameState->currPlayer);

    if (player == gameState->leadPlayer) {
        DEBUG_PRINTF("setting lead suit %c\n", card.suit);
        gameState->leadSuit = card.suit;
    }
    assert(0 <= player && player < gameState->table->numCards);
    gameState->table->cards[player] = card;

    assert(gameState->currPlayer == player);
    gameState->currPlayer = player;
    gameState->currPlayer++;
    gameState->currPlayer %= gameState->numPlayers;
}

// see header
void gs_end_round(GameState* gameState) {
    DEBUG_PRINT("ending round");

    int winningPlayer = deck_best_card(gameState->table,
            gameState->leadSuit, true);
    assert(winningPlayer >= 0);
    Card winningCard = gameState->table->cards[winningPlayer];
    assert(winningCard.suit == gameState->leadSuit);
    
    // count diamonds on the table.
    Deck* table = gameState->table;
    int diamonds = 0;
    for (int i = 0; i < table->numCards; i++) {
        if (table->cards[i].suit == 'D') {
            diamonds++;
        }
    }

    char cardBuf[3];
    fmt_card(cardBuf, winningCard, false);
    DEBUG_PRINTF("player %d won with card %s. won %d D\n", winningPlayer,
            cardBuf, diamonds);

    // increment points and give diamonds to winning player.
    gameState->playerPoints[winningPlayer]++;
    gameState->diamondsWon[winningPlayer] += diamonds;
    // winning player is the lead player
    gameState->leadPlayer = winningPlayer;
}

// see header
void gs_fprint_cards(GameState* gameState, FILE* file) {
    int numPlayers = gameState->numPlayers;
    int leadPlayer = gameState->leadPlayer;
    for (int i = 0; i < numPlayers; i++) {
        if (i > 0) {
            fprintf(file, " ");
        }
        Card card = gameState->table->cards[(leadPlayer + i) % numPlayers];
        fprintf(file, "%c.%d", card.suit, card.rank);
    }
    fprintf(file, "\n");
}
