#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "exitCodes.h"

// see header
const char* hub_message(HubExitCode code) {
    assert(0 <= code && code < 10);
    static const char* messages[10];
    messages[0] = "";
    messages[1] = "Usage: 2310hub deck threshold player0 {player1}\n";
    messages[2] = "Invalid threshold\n";
    messages[3] = "Deck error\n";
    messages[4] = "Not enough cards\n";
    messages[5] = "Player error\n";
    messages[6] = "Player EOF\n";
    messages[7] = "Invalid message\n";
    messages[8] = "Invalid card choice\n";
    messages[9] = "Ended due to signal\n";
    return messages[code];
}

// see header
const char* player_message(PlayerExitCode code) {
    assert(0 <= code && code < 8);
    static const char* messages[8];
    messages[0] = "";
    messages[1] = "Usage: player players myid threshold handsize\n";
    messages[2] = "Invalid players\n";
    messages[3] = "Invalid position\n";
    messages[4] = "Invalid threshold\n";
    messages[5] = "Invalid hand size\n";
    messages[6] = "Invalid message\n";
    messages[7] = "EOF\n";
    return messages[code];
}
