#include <string.h>

#include "arrayHelpers.h"
#include "material.h"
#include "connection.h"
#include "deferGroup.h"
#include "messages.h"

// compares two char*'s, passed as void*'s
int ah_strcmp(void* a, void* b) {
    return strcmp(a, b);
}
// compares the integers pointed to by two int*'s, passed as void*'s
int ah_intcmp(void* a, void* b) {
    int aVal = *(int*)a;
    int bVal = *(int*)b;
    return aVal - bVal;
}

void* ah_mat_mapper(void* material) {
    return ((Material*) material)->name;
}

void* ah_conn_mapper(void* connection) {
    return ((Connection*) connection)->name;
}

void* ah_dg_mapper(void* deferGroup) {
    // because key is stored as an int, take the pointer
    return &((DeferGroup*) deferGroup)->key;
}

void ah_mat_destroy(void* material) {
    mat_destroy(material);
}

void ah_conn_destroy(void* connection) {
    conn_destroy(connection);
}

void ah_dg_destroy(void* deferGroup) {
    dg_destroy(deferGroup);
}

void ah_msg_destroy(void* message) {
    msg_destroy(message);
}

/*
void ah_msgfrom_destroy(void* message) {
    msgfrom_destroy(message);
}
*/
