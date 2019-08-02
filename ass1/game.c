#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "game.h"
#include "util.h"
#include "exitCodes.h"
#include "scoring.h"

#define ENSURE_NONNEG(x) if (x < 0) return false;

void init_game_state(GameState* gameState) {
    gameState->currPlayer = 0;
    gameState->numDrawn = 0;
    gameState->boardState = NULL;
    gameState->deck = NULL;
    for (int i = 0; i < NUM_HAND*NUM_PLAYERS; i++) {
        gameState->playerHands[i] = NULL_CARD;
    }
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
    free(indexes);
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
    return true;
}

Card* get_hand(GameState* gameState, int playerIndex) {
    return gameState->playerHands + NUM_HAND*playerIndex; // get_hand()
}

Card* get_player_hand(GameState* gameState) {
    return get_hand(gameState, gameState->currPlayer);
}

bool parse_all_hands(FILE* file, GameState* gameState) {
    int currPlayer = gameState->currPlayer;
    for (int playerIndex = 0; playerIndex < NUM_PLAYERS; playerIndex++) {
        Card* hand = get_hand(gameState, playerIndex);
        // the current player is expected to have 1 more than others.
        int expectedCards = NUM_HAND - (playerIndex==currPlayer ? 0 : 1);
        if (!parse_card_row(file, hand, expectedCards, false)) {
            DEBUG_PRINT("invalid player hand");
            return false;
        }
    }
    return true;
}

bool load_game_file(GameState* gameState, char* saveFile) {
    FILE* file = fopen(saveFile, "r");
    if (file == NULL) {
        DEBUG_PRINT("error opening savefile");
        return false;
    }
    int w, h, n, v;
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

void print_hand(GameState* gameState) {
    Card* hand = get_player_hand(gameState);
    for (int i = 0; i < NUM_HAND; i++) {
        Card card = hand[i];
        if (is_null_card(card)) {
            continue;
        }
        char str[3];
        printf("%s ", fmt_card(str, card));
    }
    printf("\n");
}

bool deal_cards(GameState* gameState) {
    // draw cards for each players sequentially, not alternating players
    for (int p = 0; p < NUM_PLAYERS; p++) {
        DEBUG_PRINT("distributing cards");
        Card* hand = get_hand(gameState, p);
        // draws NUM_HAND-1 actual cards because no player is 'playing' yet
        for (int i = 0; i < NUM_HAND; i++) {
            if (i == NUM_HAND-1) { // set last card to null.
                hand[i] = NULL_CARD;
                continue;
            }
            Card card = draw_card(gameState);;
            if (is_null_card(card)) {
                return false;
            }
            hand[i] = card;
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

bool save_game_file(GameState* gameState, char* saveFile) {
    GameState* gs = gameState;
    BoardState* bs = gs->boardState;
    DEBUG_PRINTF("attempting to save to |%s|\n", saveFile);
    FILE* file = fopen(saveFile, "w");
    if (file == NULL) {
        return false;
    }
    ENSURE_NONNEG(fprintf(file, "%d %d %d %d\n%s\n", bs->width, bs->height,
            gs->numDrawn, gs->currPlayer+1, gs->deckFile));
    char str[3];
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < NUM_HAND; i++) {
            Card card = get_hand(gs, p)[i];
            if (is_null_card(card)) {
                break;
            }
            ENSURE_NONNEG(fprintf(file, "%s", fmt_card(str, card)));
        }
        ENSURE_NONNEG(fprintf(file, "\n"));
    }
    int w = bs->width;
    int h = bs->height;
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            Card card = bs->board[w*r + c];
            ENSURE_NONNEG(fprintf(file, "%s",
                    fmt_card_c(str, card, BLANK_CHAR_SAVED)));
        }
        ENSURE_NONNEG(fprintf(file, "\n"));
    }
    DEBUG_PRINT("closing file");
    fclose(file);
    return true;
}

void remove_card_from_hand(GameState* gameState, int cardNum) {
    Card* hand = get_player_hand(gameState);
    for (int i = cardNum; i < NUM_HAND-1; i++) {
        hand[i] = hand[i+1]; // shuffle subsequent cards to the left.
    }
    hand[NUM_HAND-1] = NULL_CARD; // insert null card at end.
}

bool prompt_move(GameState* gameState) {
    GameState* gs = gameState;
    char* input = NULL;
    while (1) {
        free(input); // free input line from previous prompt.
        input = NULL;
        printf("Move? ");
        fflush(stdout);
        if (!safe_read_line(stdin, &input) || feof(stdin)) {
            DEBUG_PRINT("error reading human input");
            free(input);
            return false;
        }
        if (strncmp(input, "SAVE", 4) == 0) {
            // must have > 4 chars. skip "SAVE" to get filename.
            if (strlen(input) <= 4 || !save_game_file(gameState, input+4)) {
                printf("Unable to save\n");
            }
            continue;
        }
        int* indexes;
        int numTokens = tokenise(input, &indexes);
        if (numTokens != 3) {
            DEBUG_PRINT("invalid number of tokens");
            free(indexes);
            continue;
        }
        int cardNum = parse_int(input+indexes[0])-1; // shift to 0-indexed
        int col = parse_int(input+indexes[1])-1;
        int row = parse_int(input+indexes[2])-1;
        free(indexes);
        free(input);
        input = NULL;
        if (cardNum < 0 || cardNum >= NUM_HAND
                || !is_on_board(gs->boardState, row, col)) {
            DEBUG_PRINT("card or row/col number outside of range");
            continue;
        }
        Card card = get_player_hand(gs)[cardNum];
        assert(!is_null_card(card)); // player should always have 6 cards.
        if (!place_card(gs->boardState, row, col, card)) {
            DEBUG_PRINT("cannot put card here");
            continue;
        }
        remove_card_from_hand(gameState, cardNum);
        return true;
    }
}

void finish_auto_turn(GameState* gameState, Card card, int row, int col) {
    char str[3];
    printf("Player %d plays %s in column %d row %d\n",
            gameState->currPlayer+1, fmt_card(str, card), col+1, row+1);
    remove_card_from_hand(gameState, 0); // assuming always plays first card
}

void play_auto_turn(GameState* gameState) {
    GameState* gs = gameState;
    BoardState* bs = gameState->boardState;
    int w = bs->width;
    int h = bs->height;
    DEBUG_PRINT("playing auto turn");
    Card card = get_player_hand(gs)[0]; // get first card
    if (is_board_empty(bs)) {
        int r = (h+1)/2-1;
        int c = (w+1)/2-1;
        if (!place_card(bs, r, c, card)) {
            DEBUG_PRINT("catastrophic failure placing first auto card");
        }
        finish_auto_turn(gs, card, r, c);
        return;
    }
    bool p1 = gameState->currPlayer % 2 == 0; // generalise to N players
    // generalising the for loops based on their start and the direction they
    // iterate in.
    int startRow = p1 ? 0 : h-1;
    int dr = p1 ? 1 : -1;
    int startCol = p1 ? 0 : w-1;
    int dc = p1 ? 1 : -1;
    for (int r = startRow; 0 <= r && r < h; r += dr) {
        for (int c = startCol; 0 <= c && c < w; c += dc) {
            if (place_card(bs, r, c, card)) {
                finish_auto_turn(gs, card, r, c);
                return;
            }
        }
    }
    DEBUG_PRINT("failed to play move. should never happen.");
}

void print_points(GameState* gameState) {
    int letterLengths[NUM_LETTERS]; // length of longest path of each letter.
    longest_letter_paths(gameState->boardState, letterLengths);

    int playerPoints[NUM_PLAYERS]; // longest path for each player.
    memset(playerPoints, 0, sizeof(int)*NUM_PLAYERS); // initialise to 0
    for (int l = 0; l < NUM_LETTERS; l++) {
        int p = l % NUM_PLAYERS; // p = 0 when letter is A (0)
        if (letterLengths[l]+1 > playerPoints[p]) {
            // IMPORTANT: score is path length + 1 (i.e. number of cards)
            playerPoints[p] = letterLengths[l]+1;
        }
    }
    for (int p = 0; p < NUM_PLAYERS; p++) {
        if (p > 0) {
            printf(" ");
        }
        printf("Player %d=%d", p+1, playerPoints[p]);
    }
    printf("\n");
}

int exec_game_loop(GameState* gameState, char* playerTypes) {
    DEBUG_PRINTF("starting game loop with player types %c %c\n",
            playerTypes[0], playerTypes[1]);
    GameState* gs = gameState;
    while (1) {
        print_board(gs->boardState);
        if (is_board_full(gs->boardState)) {
            break;
        }
        Card* playerHand = get_player_hand(gs);
        // if currPlayer has <6 cards, draw one more card.
        if (is_null_card(playerHand[NUM_HAND-1])) {
            Card drawnCard = draw_card(gs);
            if (is_null_card(drawnCard)) {
                break; // no more cards in deck. exit normally.
            }
            // assumes there will be either NUM_HAND or NUM_HAND-1 cards
            // at all times.
            playerHand[NUM_HAND-1] = drawnCard;
        }
        bool isHuman = playerTypes[gs->currPlayer] == 'h';
        if (isHuman) {
            printf("Hand(%d): ", gs->currPlayer+1);
            print_hand(gs);
            if (!prompt_move(gameState)) {
                return EXIT_EOF;
            }
        } else {
            printf("Hand: ");
            print_hand(gs);
            play_auto_turn(gs);
        }
        gs->currPlayer = (gs->currPlayer+1) % NUM_PLAYERS;
    }
    DEBUG_PRINT("game ended, computing points");
    print_points(gs);
    return EXIT_SUCCESS;
}
