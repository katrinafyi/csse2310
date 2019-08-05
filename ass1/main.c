#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "deck.h"
#include "board.h"
#include "exitCodes.h"
#include "util.h"

// barely exceeds 50 lines. pls forgive, this bootstraps everything.
int exec_main(int argc, char** argv) {
    if (!(argc == 4 || argc == 6)) {
        return EXIT_INCORRECT_ARGS;
    }
    int isNewGame = argc == 6 ? 1 : 0;
    char playerTypes[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        // type arguments are 4,5 for new game and 2,3 for save file.
        char* typeArg = argv[2 + i + 2 * isNewGame];
        // recall that strcmp returns 0 on equal
        if (!(strcmp(typeArg, "a") == 0 || strcmp(typeArg, "h") == 0)) {
            return EXIT_INCORRECT_ARG_TYPES;
        }
        playerTypes[i] = *typeArg; // grab first char of arg.
    }
    GameState gameState; // store structs on the stack
    BoardState boardState;
    Deck deck;
    init_game_state(&gameState); // initialises gameState struct
    gameState.deck = &deck;
    gameState.boardState = &boardState;
    if (!isNewGame) { // loading save file.
        if (!load_game_file(&gameState, argv[1])) {
            return EXIT_SAVE_ERROR;
        }
        if (is_board_full(&boardState)) {
            return EXIT_BOARD_FULL;
        }
    } else { // else, we are starting a new game.
        int w = parse_int(argv[2]);
        int h = parse_int(argv[3]);
        if (!is_size_valid(w, h)) {
            return EXIT_INCORRECT_ARG_TYPES;
        }
        gameState.deckFile = argv[1];
        init_board(gameState.boardState, w, h);
    }
    if (!load_deck_file(&deck, gameState.deckFile)) {
        return EXIT_DECK_ERROR;
    }
    if (isNewGame && !deal_cards(&gameState)) { // draws first 10 cards
        return EXIT_DECK_SHORT;
    }
    int ret = exec_game_loop(&gameState, playerTypes);
    // if new game, deckFile is on the stack and shouldn't be freed.
    free(!isNewGame ? gameState.deckFile : NULL);
    free(boardState.board);   // obviously if we returned earlier, these
    free(deck.cards);         // would not be freed.
    return ret;
}

int main(int argc, char** argv) {
    int ret = exec_main(argc, argv);
    char* error = "";
    switch (ret) {
        case EXIT_SUCCESS:
            break;
        case EXIT_INCORRECT_ARGS:
            error = "Usage: bark savefile p1type p2type\n"
                    "bark deck width height p1type p2type\n";
            break;
        case EXIT_INCORRECT_ARG_TYPES:
            error = "Incorrect arg types\n";
            break;
        case EXIT_DECK_ERROR:
            error = "Unable to parse deckfile\n";
            break;
        case EXIT_SAVE_ERROR:
            error = "Unable to parse savefile\n";
            break;
        case EXIT_DECK_SHORT:
            error = "Short deck\n";
            break;
        case EXIT_BOARD_FULL:
            error = "Board full\n";
            break;
        case EXIT_EOF:
            error = "End of input\n";
            break;
        default:
            error = "UNKNOWN EXIT CODE\n";
    }
    fprintf(stderr, "%s", error);
    DEBUG_PRINTF("exiting with code %d\n", ret);
    return ret;
}
