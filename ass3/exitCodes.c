#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "exitCodes.h"

void print_hub_message(HubExitCode code) {
    char* str = NULL;
    switch (code) {
        case H_NORMAL:
            break;
        case H_INCORRECT_ARGS:
            str = "Usage: 2310hub deck threshold player0 {player1}";
            break;
        case H_INCORRECT_THRESHOLD:
            str = "Invalid threshold";
            break;
        case H_DECK_ERROR:
            str = "Deck error";
            break;
        case H_DECK_SHORT:
            str = "Not enough cards";
            break;
        case H_PLAYER_ERROR:
            str = "Player error";
            break;
        case H_PLAYER_EOF:
            str = "Player EOF";
            break;
        case H_INVALID_MESSAGE:
            str = "Invalid message";
            break;
        case H_INVALID_CARD:
            str = "Invalid card choice";
            break;
        case H_SIGNAL:
            str = "Ended due to signal";
            break;
        default:
            assert(0);
    }
    if (str != NULL) {
        fprintf(stderr, "%s\n", str);
    }
}

void print_player_message(PlayerExitCode code) {
    char* str = NULL;
    switch (code) {
        case P_NORMAL:
            break;
        case P_INCORRECT_ARGS:
            str = "Usage: player players myid threshold handsize";
            break;
        case P_INCORRECT_PLAYERS:
            str = "Invalid players";
            break;
        case P_INCORRECT_POSITION:
            str = "Invalid position";
            break;
        case P_INCORRECT_THRESHOLD:
            str = "Invalid threshold";
            break;
        case P_INCORRECT_HAND:
            str = "Invalid hand size";
            break;
        case P_INVALID_MESSAGE:
            str = "Invalid message";
            break;
        case P_HUB_EOF:
            str = "EOF";
            break;
        default:
            assert(0);
    }
    if (str != NULL) {
        fprintf(stderr, "%s\n", str);
    }
}
