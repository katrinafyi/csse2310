#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "material.h"
#include "util.h"

// see header
void mat_init(Material* material, int quantity, char* name) {
    material->quantity = quantity;
    material->name = strdup(name); // malloc's copy of name
}

// see header
void mat_destroy(Material* material) {
    if (material == NULL) {
        return;
    }
    TRY_FREE(material->name);
}

