#ifndef EXITCODES_H
#define EXITCODES_H

#define NUM_EXIT_CODES 4

/* Exit codes for the depot, defined in spec. */
typedef enum DepotExitCode {
    D_NORMAL = 0,
    D_INCORRECT_ARGS = 1,
    D_INVALID_NAME = 2,
    D_INVALID_QUANTITY = 3
} DepotExitCode;

/* Returns the depot error message associated with the given code.
 * Returns a statically allocated string.
 */
const char* depot_message(DepotExitCode code);

#endif
