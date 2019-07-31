#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "deck.h"
#include "board.h"
#include "exitCodes.h"
#include "util.h"

int main(int argc, char** argv) {
    if (!(argc == 4 || argc == 6)) {
        // invalid argument lengths.
        fprintf(stderr, "Usage: bark savefile p1type p2type\n"
            "bark deck width height p1type p2type\n");
        return EXIT_INCORRECT_ARGS;
    }

    int isNewGame = argc == 6 ? 1 : 0;
    char playerTypes[2];
    for (int a = 0; a < 2; a++) {
        char* typeArg = argv[2 + a + 2*isNewGame];
        // recall that strcmp returns 0 on equal
        if (strcmp(typeArg, "a") && strcmp(typeArg, "h")) {
            fprintf(stderr, "Incorrect arg types\n");
            return EXIT_INCORRECT_ARG_TYPES;
        }
        playerTypes[a] = *typeArg;
    }

    GameState gameState;
    Deck deck;
    init_game_state(&gameState);
    gameState.deck = &deck;
    if (!isNewGame && !load_game_file(&gameState, argv[1])) {
        fprintf(stderr, "Unable to parse savefile\n");
        return EXIT_SAVE_ERROR;
    }
    if (!load_deck_file(&deck, isNewGame ? argv[1] : gameState.deckFile)) {
        fprintf(stderr, "Unable to parse deckfile\n");
        return EXIT_DECK_ERROR;
    }
    printf("numCards %d\n", deck.numCards);
    for (int i = 0; i<deck.numCards; i++) {
        char* cardStr = fmt_card(deck.cards[i]);
        printf("card %d is %s\n", i, cardStr);
    }

    char* a;
    char* b2;
    printf("card: %s\n", a = fmt_card((Card) {8, '$'}));
    printf("card: %s\n", b2 = fmt_card_c((Card) {0, '$'}, '*'));
    free(a);
    free(b2);

    BoardState b = {0};
    init_board(&b, 10, 10);
    print_board(&b);
    for (int i = 0; i < 10; i++)
        place_card(&b, i, i, (Card) {9, 'A'+i});
    printf("\n\n");
    print_board(&b);

    free(b.board);
    return 0;
    
}
