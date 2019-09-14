#ifndef EXITCODES_H
#define EXITCODES_H

/* Exit codes for the hub. */
typedef enum {
    NORMAL = 0,
    INCORRECT_ARGS = 1,
    INCORRECT_THRESHOLD = 2,
    DECK_ERROR = 3,
    DECK_SHORT = 4,
    PLAYER_ERROR = 5,
    PLAYER_EOF = 6,
    INVALID_MESSAGE = 7,
    INVALID_CARD = 8,
    SIGNAL = 9,
} HubExitCode;

/* Exit codes for the player. */
typedef enum {
    NORMAL = 0,
    INCORRECT_ARGS = 1,
    INCORRECT_PLAYERS = 2,
    INCORRECT_POSITION = 3,
    INCORRECT_THRESHOLD = 4,
    INCORRECT_HAND = 5,
    INVALID_MESSAGE = 6,
    HUB_EOF = 7,
} PlayerExitCode;

#endif
