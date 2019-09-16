#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "exitCodes.h"

// see header
void print_hub_message(HubExitCode code) {
    assert(0 <= code && code < 10);
    static char* messages[10];
    messages[0] = NULL;
    messages[1] = "Usage: 2310hub deck threshold player0 {player1}";
    messages[2] = "Invalid threshold";
    messages[3] = "Deck error";
    messages[4] = "Not enough cards";
    messages[5] = "Player error";
    messages[6] = "Player EOF";
    messages[7] = "Invalid message";
    messages[8] = "Invalid card choice";
    messages[9] = "Ended due to signal";
    if (messages[code] != NULL) {
        fprintf(stderr, "%s\n", messages[code]);
    }
}

// see header
void print_player_message(PlayerExitCode code) {
    assert(0 <= code && code < 8);
    static char* messages[8];
    messages[0] = NULL;
    messages[1] = "Usage: player players myid threshold handsize";
    messages[2] = "Invalid players";
    messages[3] = "Invalid position";
    messages[4] = "Invalid threshold";
    messages[5] = "Invalid hand size";
    messages[6] = "Invalid message";
    messages[7] = "EOF";
    if (messages[code] != NULL) {
        fprintf(stderr, "%s\n", messages[code]);
    }
}
