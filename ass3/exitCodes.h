#ifndef EXITCODES_H
#define EXITCODES_H

/* Exit codes for the hub, as defined in spec. */
typedef enum HubExitCode {
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

/* Exit codes for the player, as defined in spec. */
typedef enum PlayerExitCode {
    P_NORMAL = 0,
    P_INCORRECT_ARGS = 1,
    P_INCORRECT_PLAYERS = 2,
    P_INCORRECT_POSITION = 3,
    P_INCORRECT_THRESHOLD = 4,
    P_INCORRECT_HAND = 5,
    P_INVALID_MESSAGE = 6,
    P_HUB_EOF = 7,
} PlayerExitCode;

/* Returns the hub message associated with the given exit code.
 * Returns a statically allocated string.
 */
const char* hub_message(HubExitCode code);

/* Returns the player message associated with the given exit code.
 * Returns a statically allocated string.
 */
const char* player_message(PlayerExitCode code);

#endif
