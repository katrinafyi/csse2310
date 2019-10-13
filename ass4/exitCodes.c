#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "exitCodes.h"

// see header
const char* depot_message(DepotExitCode code) {
    assert(0 <= code && code < NUM_EXIT_CODES);
    const char* messages[NUM_EXIT_CODES];

    messages[D_NORMAL] = "";
    messages[D_INCORRECT_ARGS] = "Usage: 2310depot name {goods qty}\n";
    messages[D_INVALID_NAME] = "Invalid name(s)\n";
    messages[D_INVALID_QUANTITY] = "Invalid quantity\n";
    // string literals are always static
    return messages[code];
}
