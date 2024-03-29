#ifndef DEFERGROUP_H
#define DEFERGROUP_H

#include "array.h"
#include "messages.h"

typedef struct DeferGroup {
    int key;
    Array* messages;
} DeferGroup;

/* Initialises a defer group with the given defer key.
 */
void dg_init(DeferGroup* deferGroup, int key);

/* Destroys a defer group and frees its memory.
 */ 
void dg_destroy(DeferGroup* deferGroup);

/* Appends the given message pointer to the messages of this group.
   Message should be a MALLOC'd pointer, this takes ownership of the pointer.
 */
void dg_add_message(DeferGroup* deferGroup, Message* message);

#endif
