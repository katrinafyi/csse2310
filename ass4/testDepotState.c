#include <stdio.h>

#include "depotState.h"
#include "material.h"

// prints details about the wood resource stored in the given array.
void print_wood(Array* materials) {
    Material* mat = arraymap_get(materials, "wood");
    printf("mat at %p: %d, %s\n", (void*)mat, mat->quantity, mat->name);
}

// prints a material in the array.
void print_mat(void* voidMat) {
    Material* mat = voidMat;
    printf("mat: %s, %d\n", mat->name, mat->quantity);
}

// testing depot state functions
int main(int argc, char** argv) {
    DepotState depotStateData = {0};
    DepotState* depotState = &depotStateData;
    ds_destroy(depotState); // test destroy on {0}

    ds_init(depotState);
    
    ARRAY_WRLOCK(depotState->materials);

    // test adding wood with 0
    ds_ensure_mat(depotState, "wood");
    print_wood(depotState->materials);
    // test incrementing wood
    ds_alter_mat(depotState, "wood", +125);
    print_wood(depotState->materials);

    // test decrementing
    ds_alter_mat(depotState, "wood", -25);
    print_wood(depotState->materials);

    array_foreach(depotState->materials, print_mat);

    ARRAY_UNLOCK(depotState->materials);

    ds_destroy(depotState);
}
