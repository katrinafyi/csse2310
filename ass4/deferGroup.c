#include <stdlib.h>

#include "deferGroup.h"
#include "array.h"
#include "util.h"
#include "arrayHelpers.h"

// see header
void dg_init(DeferGroup* deferGroup, int key) {
    deferGroup->key = key;
    deferGroup->messages = calloc(1, sizeof(Array));
    array_init(deferGroup->messages);
}

// see header
void dg_destroy(DeferGroup* deferGroup) {
    if (deferGroup == NULL) {
        return;
    }
    if (deferGroup->messages != NULL) {
        array_foreach(deferGroup->messages, ah_msg_destroy);
    }
    array_destroy_and_free(deferGroup->messages);
    TRY_FREE(deferGroup->messages);
}

// see header
void dg_add_message(DeferGroup* deferGroup, Message message) {
    DEBUG_PRINTF("adding message to defer key: %d\n", deferGroup->key);
    array_add_copy(deferGroup->messages, &message, sizeof(Message));
}

