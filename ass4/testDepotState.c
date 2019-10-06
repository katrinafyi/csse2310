#include <stdio.h>

#include "depotState.h"
#include "material.h"

// testing depot state functions
int main(int argc, char** argv) {
    DepotState depotStateData = {0};
    DepotState* depotState = &depotStateData;

    ds_init(depotState);
    
    ARRAY_WRLOCK(depotState->materials);

    ds_ensure_mat(depotState, "wood");
    Material* mat = arraymap_get(depotState->materials, "wood");
    printf("mat at %p: %d, %s\n", mat, mat->quantity, mat->name);

    ds_alter_mat(depotState, "wood", +125);

    mat = arraymap_get(depotState->materials, "wood");
    printf("mat at %p: %d, %s\n", mat, mat->quantity, mat->name);

    ARRAY_UNLOCK(depotState->materials);

    ds_destroy(depotState);
}
