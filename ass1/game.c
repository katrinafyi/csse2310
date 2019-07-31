#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "util.h"

void init_game_state(GameState* gameState) {
    gameState->currPlayer = 0;
    gameState->numDrawn = 0;
    gameState->deck = NULL;
    gameState->boardState = malloc(sizeof(BoardState));
}

bool parse_top_line(FILE* file, int* w, int* h, int* n, int* v) {
    char* topLine;
    if (!safe_read_line(file, &topLine)) {
        printf("error top line\n");
        return false;
    }
    // note that we INCLUDE the trailing \0 in the for loop
    // safe_read_line() guarantees our line contains no \0
    // in the middle.
    int start = 0;
    int topLineNums[4];
    int num = 0;
    // strlen() will be wrong once we start inserting \0's
    int fullLength = strlen(topLine); 
    for (int i = 0; i <= fullLength; i++) {
        char c = topLine[i];
        // if we have whitespace, treat this point
        // as the end of the string and pass to parse_int().
        if (c == ' ' || c == '\0') {
            topLine[i] = '\0';
            int parsed = parse_int(topLine+start);
            if (parsed < 0 || num >= 4) {
                DEBUG_PRINT("invalid integer top line");
                free(topLine);
                return false;
            }
            topLineNums[num] = parsed;
            num++;
            // next int starts from the char after the space.
            start = i+1;
        }
    }
    free(topLine);
    if (num < 4) {
        DEBUG_PRINT("insufficient ints");
        return false;
    }
    *w = topLineNums[0];
    *h = topLineNums[1];
    *n = topLineNums[2];
    *v = topLineNums[3];
    return true;
}

bool parse_card_row(FILE* file, Card* cards, int numExpected) {
    char* line;
    // ensure line length is even.
    if (!safe_read_line(file, &line) || strlen(line) % 2 == 1) {
        return false;
    }
    int i;
    int num = 0;
    // iterate in blocks of 2 characters.
    for (i = 0; i < strlen(line); i += 2) {
        if (!is_card(line+i) || num >= numExpected) {
            free(line);
            return false;
        }
        cards[num] = to_card(line+i);
        num++;
    }
    free(line);
    if (num < numExpected) {
        return false;
    }
    // fill in the remaining cards in the hand as
    // null cards.
    for (; num < NUM_HAND; num++) {
        cards[num] = NULL_CARD;
    }
    return true;
}

bool parse_all_hands(FILE* file, GameState* gameState) {
    int currPlayer = gameState->currPlayer;
    for (int playerIndex = 0; playerIndex < NUM_PLAYERS; playerIndex++) {
        Card* playerHand = gameState->playerHands + NUM_HAND*playerIndex;
        int expectedCards = NUM_HAND - (playerIndex==currPlayer ? 0 : 1);
        if (!parse_card_row(file, playerHand, expectedCards)) {
            DEBUG_PRINT("invalid player hand");
            return false;
        }
        print_hand(gameState, playerIndex);
    }
    return true;
}

bool load_game_file(GameState* gameState, char* saveFile) {
    FILE* file = fopen(saveFile, "r");
    if (file == NULL) {
        DEBUG_PRINT("error opening savefile");
        return false;
    }
    int w;
    int h;
    int n;
    int v;
    if (!parse_top_line(file, &w, &h, &n, &v)) {
        return false;
    }
    gameState->numDrawn = n;
    gameState->currPlayer = v;
    DEBUG_PRINT("top line parsed");
    if (!safe_read_line(file, &(gameState->deckFile))) {
        return false;
    }
    // printf("deck file is %s\n", gameState->deckFile);
    if (!parse_all_hands(file, gameState)) {
        return false;
    }
    DEBUG_PRINT("init board");
    init_board(gameState->boardState, w, h);
    Card* boardCards = gameState->boardState->board;
    for (int row = 0; row < h; row++) {
        DEBUG_PRINT("row parsing");
        if (!parse_card_row(file, boardCards+row*w, w)) {
            DEBUG_PRINT("invalid board row");
            return false;
        }
    }
    print_board(gameState->boardState);
    if (fgetc(file) != EOF) {
        DEBUG_PRINT("extra junk at eof");
        return false;
    }
    DEBUG_PRINT("game file load done");

}

void print_hand(GameState* gameState, int playerIndex) {
    Card* firstCard = gameState->playerHands + NUM_HAND*playerIndex;
    for (int i = 0; i < NUM_HAND; i++) {
        Card card = firstCard[i];
        if (card.num == 0) {
            continue;
        }
        char* str = fmt_card(card);
        printf("%d: %s\n", i, str);
        free(str);
    }
}
