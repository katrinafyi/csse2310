#include <string.h>

#include "arrayHelpers.h"
#include "material.h"
#include "connection.h"
#include "deferGroup.h"
#include "messages.h"

// see header
int ah_strcmp(void* a, void* b) {
    return strcmp(a, b);
}

// see header
int ah_intcmp(void* a, void* b) {
    int aVal = *(int*)a;
    int bVal = *(int*)b;
    return aVal - bVal;
}

// see header
void* ah_mat_mapper(void* material) {
    return ((Material*) material)->name;
}

// see header
void* ah_conn_mapper(void* connection) {
    return ((Connection*) connection)->name;
}

// see header
void* ah_dg_mapper(void* deferGroup) {
    // because key is stored as an int, take the pointer
    return &((DeferGroup*) deferGroup)->key;
}

// see header
void ah_mat_destroy(void* material) {
    mat_destroy(material);
}

// see header
void ah_conn_destroy(void* connection) {
    conn_destroy(connection);
}

// see header
void ah_dg_destroy(void* deferGroup) {
    dg_destroy(deferGroup);
}

// see header
void ah_msg_destroy(void* message) {
    msg_destroy(message);
}

