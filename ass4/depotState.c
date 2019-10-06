#include <stdio.h>
#include <assert.h>

#include "depotState.h"
#include "material.h"
#include "array.h"
#include "util.h"
#include "arrayHelpers.h"

// see header
void ds_init(DepotState* depotState) {
    // array of Material
    depotState->materials = calloc(1, sizeof(Array));
    arraymap_init(depotState->materials, ah_mat_mapper, ah_strcmp);

    // array of Connection
    depotState->connections = calloc(1, sizeof(Array));

    // array of DeferGroup
    depotState->deferGroups = calloc(1, sizeof(Array));
}

// see header
void ds_destroy(DepotState* depotState) {
    Array* mats = depotState->materials;
    if (mats != NULL) {
        array_foreach(mats, ah_mat_destroy);
        array_destroy_and_free(mats);
    }
    TRY_FREE(depotState->materials);
    
    array_destroy_and_free(depotState->connections);
    TRY_FREE(depotState->connections);

    array_destroy_and_free(depotState->deferGroups);
    TRY_FREE(depotState->deferGroups);
}

// see header
void ds_add_depot(DepotState* depotState, int port, char* name) {
    assert(0);
}

// see header
void ds_ensure_mat(DepotState* depotState, char* matName) {
    Material mat;
    mat_init(&mat, 0, matName);
    DEBUG_PRINTF("adding empty material: %s\n", matName);
    array_add_copy(depotState->materials, &mat, sizeof(Material));
}

// see header
void ds_alter_mat(DepotState* depotState, char* matName, int delta) {
    ds_ensure_mat(depotState, matName);

    Array* materials = depotState->materials;
    Material* mat = arraymap_get(materials, matName);
    assert(mat != NULL);
    mat->quantity += delta;
}
