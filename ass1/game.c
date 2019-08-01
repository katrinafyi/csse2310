#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "util.h"
#include "exitCodes.h"
#include "longestPath.h"

void init_game_state(GameState* gameState) {
    gameState->currPlayer = 0;
    gameState->numDrawn = 0;
    gameState->deck = NULL;
    gameState->boardState = malloc(sizeof(BoardState));
    gameState->deck = malloc(sizeof(Deck));
}

bool parse_top_line(FILE* file, int* w, int* h, int* n, int* v) {
    char* topLine;
    if (!safe_read_line(file, &topLine)) {
        DEBUG_PRINT("error top line of savefile");
        return false;
    }
    // note that we INCLUDE the trailing \0 in the for loop
    // safe_read_line() guarantees our line contains no \0
    // in the middle.
    int topLineNums[4];
    int* indexes;
    int numTokens = tokenise(topLine, &indexes);
    if (numTokens != 4) {
        DEBUG_PRINT("not exactly 4 ints");
        return false;
    }
    for (int i = 0; i < numTokens; i++) {
        int parsed = parse_int(topLine+indexes[i]);
        if (parsed < 0) {
            DEBUG_PRINT("invalid integer top line");
            free(topLine);
            return false;
        }
        topLineNums[i] = parsed;
    }
    free(topLine);
    *w = topLineNums[0];
    *h = topLineNums[1];
    *n = topLineNums[2];
    *v = topLineNums[3];
    return true;
}

bool parse_card_row(FILE* file, Card* cards, int numExpected,
        bool hasBlanks) {
    char* line;
    // ensure line length is even.
    if (!safe_read_line(file, &line) || strlen(line) % 2 == 1) {
        return false;
    }
    int i;
    int num = 0;
    // iterate in blocks of 2 characters.
    for (i = 0; i < strlen(line); i += 2) {
        bool valid = is_card(line+i) || (hasBlanks && is_blank(line+i));
        if (!valid || num >= numExpected) {
            free(line);
            return false;
        }
        cards[num] = is_card(line+i) ? to_card(line+i) : NULL_CARD;
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
        // the current player is expected to have 1 more than others.
        int expectedCards = NUM_HAND - (playerIndex==currPlayer ? 0 : 1);
        if (!parse_card_row(file, playerHand, expectedCards, false)) {
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
    if (!is_size_valid(w, h) || n <= 0 || v <= 0 || v > NUM_PLAYERS) {
        DEBUG_PRINT("integer out of range");
        return false;
    }
    gameState->numDrawn = n;
    gameState->currPlayer = v-1; // shift to 0-indexed
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
        if (!parse_card_row(file, boardCards+row*w, w, true)) {
            DEBUG_PRINT("invalid board row");
            return false;
        }
    }
    if (fgetc(file) != EOF) {
        DEBUG_PRINT("extra junk at eof");
        return false;
    }
    DEBUG_PRINT("game file load done");
    fclose(file);
    return true;
}

void print_hand(GameState* gameState, int playerIndex) {
    Card* firstCard = gameState->playerHands + NUM_HAND*playerIndex;
    for (int i = 0; i < NUM_HAND; i++) {
        Card card = firstCard[i];
        if (card.num == 0) {
            break;
        }
        char* str = fmt_card(card);
        if (i > 0) {
            printf(" ");
        }
        printf("%s", str);
        free(str);
    }
    printf("\n");
}

bool deal_cards(GameState* gameState) {
    for (int p = 0; p < NUM_PLAYERS; p++) {
        DEBUG_PRINT("distributing cards");
        Card* hand = gameState->playerHands + p*NUM_HAND;
        // note NUM_HAND-1 because no player is 'playing' yet
        for (int i = 0; i < NUM_HAND; i++) {
            if (i == NUM_HAND-1) { // set last card to null.
                *(hand+i) = NULL_CARD;
                continue;
            }
            Card card = draw_card(gameState);;
            DEBUG_PRINTF("card %s\n", fmt_card(card));
            if (card.num == 0) {
                return false;
            }
            *(hand+i) = card;
        }
    }
    return true;
}

Card draw_card(GameState* gameState) {
    int n = gameState->numDrawn;
    DEBUG_PRINTF("drawing the %d-th card\n", n);
    if (gameState->numDrawn >= gameState->deck->numCards) {
        DEBUG_PRINT("no more cards");
        return NULL_CARD;
    }
    gameState->numDrawn++;
    return gameState->deck->cards[n];
}



bool prompt_move(GameState* gameState) {
    while (1) {
        printf("Move? ");
        fflush(stdout);
        char* input;
        if (!safe_read_line(stdin, &input) || feof(stdin)) {
            DEBUG_PRINT("error reading human input")
            return false;
        }
        return true;
    }
}

int exec_game_loop(GameState* gameState, char* playerTypes) {
    DEBUG_PRINTF("starting game loop with player types %c %c\n", playerTypes[0], playerTypes[1]);
    // print_hand(gameState, 0);
    // print_hand(gameState, 1);
    GameState* gs = gameState;
    while (1) {
        print_board(gs->boardState);
        DEBUG_PRINTF("longest path %d\n", compute_longest_path(
                    gs->boardState, 'A', (Position) {3, 1}, 0));
        if (is_board_full(gs->boardState)) {
            break;
        }
        bool isHuman = playerTypes[gs->currPlayer] == 'h';
        if (isHuman) {
            printf("Hand(%d): ", gs->currPlayer+1);
            print_hand(gs, gs->currPlayer);
            if (!prompt_move(gameState)) {
                return EXIT_EOF;
            }
        } else {
            printf("Hand: ");
            print_hand(gs, gs->currPlayer);
        }
        printf("getchar\n");
        getchar();
        gs->currPlayer = (gs->currPlayer+1) % NUM_PLAYERS;
    }
    DEBUG_PRINT("game ended");
    return 0;
}
