#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "exitCodes.h"
#include "hubState.h"
#include "gameState.h"
#include "deck.h"
#include "util.h"


HubExitCode exec_hub_main(int argc, char** argv, HubState* hubState,
        GameState* gameState, Deck* deck) {
    if (argc < 5) {
        return H_INCORRECT_ARGS;
    }
    int threshold = parse_int(argv[2]);
    if (threshold < 2) { // includes error case -1
        return H_INCORRECT_THRESHOLD;
    }
    int numPlayers = argc - 3; // 3 non-player arguments

    if (!deck_init_file(deck, argv[1])) {
        return H_DECK_ERROR;
    }
    if (deck->numCards < numPlayers) {
        return H_DECK_SHORT;
    }

    gs_init(gameState, numPlayers, threshold);
    hs_init(hubState, gameState);

    hs_deal_cards(hubState, deck);
}

int main(int argc, char** argv) {
    GameState gameState = {0};
    HubState hubState = {0};
    Deck fullDeck = {0}; // deck of all cards to use

    HubExitCode ret = exec_hub_main(argc, argv,
            &hubState, &gameState, &fullDeck);

    gs_destroy(&gameState);
    hs_destroy(&hubState);
    deck_destroy(&fullDeck);

    print_hub_message(ret);
    return ret;
}
