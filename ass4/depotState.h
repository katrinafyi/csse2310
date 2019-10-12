#ifndef DEPOTSTATE_H
#define DEPOTSTATE_H

#include <stdlib.h>

#include "deferGroup.h"
#include "connection.h"
#include "array.h"
#include "channel.h"

/* State struct for storing the internal state of one depot, managing its own
 * resources and connections to other depots.
 */
typedef struct DepotState {
    char* name; // name of this depot, NOT malloc
    int port;

    Channel* incoming; // channel of incoming messages, as MessageFrom*
    Array* materials; // array map of materials we store, keyed by name
    Array* connections; // array map of open connections, keyed by name
    Array* deferGroups; // array map of defer groups, keyed by key
} DepotState;
// IMPORTANT: to avoid deadlocks, always lock in this order:
//     materials, connections, deferGroups
// as necessary.

/* Initialises the depot state struct, instantiating contained arrays.
 * Given name is the name of this depot.
 */
void ds_init(DepotState* depotState, char* name);

/* Destroys the depot state, freeing memory used.
 */
void ds_destroy(DepotState* depotState);

/* Adds a connection to a depot at the given port and with the given
 * name. Returns the new connection.
 */
Connection* ds_add_connection(DepotState* depotState, int port, char* name);

/* Ensures the given material name is present in our materials, adding it with
 * 0 stock if it does not exist. Returns a pointer to the material.
 */
Material* ds_ensure_mat(DepotState* depotState, char* matName);

/* Changes the given material in the depot by the given delta, which can be
 * positive or negative.
 * This ensures material is present.
 */
void ds_alter_mat(DepotState* depotState, char* matName, int delta);

/* Ensures a defer group with the given key is present in the defer group list
 * and adds it if not present. Returns a pointer to the defer group.
 */
DeferGroup* ds_ensure_defer_group(DepotState* depotState, int key);

/* Prints a goods and quantities, sorted by name and neighbours, sorted by
 * name. Format complies with SIGHUP format from spec.
 */
void ds_print_info(DepotState* depotState);
#endif
