#include <stdio.h>
#include <assert.h>

#include "depotState.h"
#include "connection.h"
#include "deferGroup.h"
#include "material.h"
#include "array.h"
#include "channel.h"
#include "util.h"
#include "arrayHelpers.h"

// see header
void ds_init(DepotState* depotState, char* name) {
    depotState->name = name;

    depotState->incoming = calloc(1, sizeof(Channel));
    chan_init(depotState->incoming);

    // array of Material, keyed by material name
    depotState->materials = calloc(1, sizeof(Array));
    arraymap_init(depotState->materials, ah_mat_mapper, ah_strcmp);

    // array of Connection, keyed by depot name
    depotState->connections = calloc(1, sizeof(Array));
    arraymap_init(depotState->connections, ah_conn_mapper, ah_strcmp);

    // array of int*'s, each pointing to a port we have an unverified connect
    // to.
    depotState->pending = calloc(1, sizeof(Array));
    arraymap_init(depotState->pending, ah_noop_mapper, ah_intcmp);

    // array of DeferGroup, keyed by defer key (as integer)
    depotState->deferGroups = calloc(1, sizeof(Array));
    arraymap_init(depotState->deferGroups, ah_dg_mapper, ah_intcmp);
}

// helper to execute the given destroy function on each item in the array
// then free the items and destroy the array.
void destroy_helper(Array* array, void (*destroy)(void*)) {
    if (array != NULL) {
        DEBUG_PRINTF("at time of destroy, array had %d items\n",
                array->numItems);
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

    if (depotState->incoming != NULL) {
        chan_foreach(depotState->incoming, ah_msg_destroy);
        chan_foreach(depotState->incoming, free);
        chan_destroy(depotState->incoming);
    }
    TRY_FREE(depotState->incoming);

    array_destroy_and_free(depotState->pending);
    TRY_FREE(depotState->pending);

    destroy_helper(depotState->materials, ah_mat_destroy);
    TRY_FREE(depotState->materials); // frees array
    
    destroy_helper(depotState->connections, ah_conn_destroy);
    TRY_FREE(depotState->connections);

    destroy_helper(depotState->deferGroups, ah_dg_destroy);
    TRY_FREE(depotState->deferGroups);
}

// see header
Connection* ds_add_connection(DepotState* depotState, int port, char* name) {
    Connection conn = {0};
    conn_init(&conn, port, name);

    DEBUG_PRINTF("adding connection to %s on %d\n", name, port);
    Connection* newConn = array_add_copy(depotState->connections, &conn, 
            sizeof(Connection));
    arraymap_sort(depotState->connections);
    return newConn;
}

// see header
Material* ds_ensure_mat(DepotState* depotState, char* matName) {
    Material* oldMat = arraymap_get(depotState->materials, matName);
    if (oldMat == NULL) {
        // material not in list
        Material mat = {0};
        mat_init(&mat, 0, matName);
        DEBUG_PRINTF("adding empty material: %s\n", matName);
        Material* newMat = array_add_copy(depotState->materials, &mat, 
                sizeof(Material));
        arraymap_sort(depotState->materials);
        return newMat;
    } else {
        return oldMat;
    }
}

// see header
void ds_alter_mat(DepotState* depotState, char* matName, int delta) {
    Material* mat = ds_ensure_mat(depotState, matName);

    DEBUG_PRINTF("changing material %s by %d\n", matName, delta);
    assert(mat != NULL);
    mat->quantity += delta;
}

// see header
DeferGroup* ds_ensure_defer_group(DepotState* depotState, int key) {
    // note & on key.
    DeferGroup* dg = arraymap_get(depotState->deferGroups, &key);
    if (dg != NULL) {
        return dg;
    }

    DEBUG_PRINTF("adding new defer group with key %d\n", key);
    DeferGroup dgNew = {0};
    dg_init(&dgNew, key);
    return array_add_copy(depotState->deferGroups, &dgNew, sizeof(DeferGroup));
}

// see header
void ds_print_info(DepotState* depotState) {
    printf("Goods:\n");
    for (int i = 0; i < depotState->materials->numItems; i++) {
        Material* mat = ARRAY_ITEM(Material, depotState->materials, i);
        // don't print materials with 0 quantity
        if (mat->quantity == 0) {
            DEBUG_PRINTF("%s %d\n", mat->name, mat->quantity);
            continue;
        }
        printf("%s %d\n", mat->name, mat->quantity);
    }

    printf("Neighbours:\n");
    for (int i = 0; i < depotState->connections->numItems; i++) {
        Connection* conn = ARRAY_ITEM(Connection, depotState->connections, i);
        printf("%s\n", conn->name);
    }
    fflush(stdout);
}
