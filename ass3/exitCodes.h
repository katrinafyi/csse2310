#ifndef EXITCODES_H
#define EXITCODES_H

/* Exit codes for the hub. */
typedef enum {
    H_NORMAL = 0,
    H_INCORRECT_ARGS = 1,
    H_INCORRECT_THRESHOLD = 2,
    H_DECK_ERROR = 3,
    H_DECK_SHORT = 4,
    H_PLAYER_ERROR = 5,
    H_PLAYER_EOF = 6,
    H_INVALID_MESSAGE = 7,
    H_INVALID_CARD = 8,
    H_SIGNAL = 9,
} HubExitCode;

/* Exit codes for the player. */
typedef enum {
    P_NORMAL = 0,
    P_INCORRECT_ARGS = 1,
    P_INCORRECT_PLAYERS = 2,
    P_INCORRECT_POSITION = 3,
    P_INCORRECT_THRESHOLD = 4,
    P_INCORRECT_HAND = 5,
    P_INVALID_MESSAGE = 6,
    P_HUB_EOF = 7,
} PlayerExitCode;

void print_hub_message(HubExitCode code);
void print_player_message(PlayerExitCode code);

#endif
