#include <stdio.h>
#include <assert.h>

#include "depotState.h"
#include "connection.h"
#include "deferGroup.h"
#include "material.h"
#include "array.h"
#include "util.h"
#include "arrayHelpers.h"

// see header
void ds_init(DepotState* depotState, char* name) {
    depotState->name = name;

    // array of Material, keyed by material name
    depotState->materials = calloc(1, sizeof(Array));
    arraymap_init(depotState->materials, ah_mat_mapper, ah_strcmp);

    // array of Connection, keyed by depot name
    depotState->connections = calloc(1, sizeof(Array));
    arraymap_init(depotState->connections, ah_conn_mapper, ah_strcmp);

    // array of DeferGroup, keyed by defer key (as integer)
    depotState->deferGroups = calloc(1, sizeof(Array));
    arraymap_init(depotState->deferGroups, ah_dg_mapper, ah_intcmp);
}

// helper to execute the given destroy function on each item in the array
// then free the items and destroy the array.
void destroy_helper(Array* array, void (*destroy)(void*)) {
    if (array != NULL) {
        // disposes each item in the array
        array_foreach(array, destroy);
        // frees memory used by each item in the array, then disposes the
        // array.
        array_destroy_and_free(array);
    }
}

// see header
void ds_destroy(DepotState* depotState) {
    if (depotState == NULL) {
        return;
    }

    destroy_helper(depotState->materials, ah_mat_destroy);
    TRY_FREE(depotState->materials); // frees array
    
    destroy_helper(depotState->connections, ah_conn_destroy);
    TRY_FREE(depotState->connections);

    destroy_helper(depotState->deferGroups, ah_dg_destroy);
    TRY_FREE(depotState->deferGroups);
}

// see header
void ds_add_connection(DepotState* depotState, int port, char* name) {
    Connection conn = {0};
    conn_init(&conn, port, name);
    DEBUG_PRINTF("adding connection to %s on %d\n", name, port);
    array_add_copy(depotState->connections, &conn, sizeof(Connection));
}

// see header
void ds_ensure_mat(DepotState* depotState, char* matName) {
    if (arraymap_get(depotState->materials, matName) == NULL) {
        // material not in list
        Material mat = {0};
        mat_init(&mat, 0, matName);
        DEBUG_PRINTF("adding empty material: %s\n", matName);
        array_add_copy(depotState->materials, &mat, sizeof(Material));
    }
}

// see header
void ds_alter_mat(DepotState* depotState, char* matName, int delta) {
    ds_ensure_mat(depotState, matName);

    DEBUG_PRINTF("changing material %s by %d\n", matName, delta);
    Array* materials = depotState->materials;
    Material* mat = arraymap_get(materials, matName);
    assert(mat != NULL);
    mat->quantity += delta;
}
