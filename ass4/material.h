#ifndef MATERIAL_H
#define MATERIAL_H

/* Material struct. Can also be used as a delta by specifying quantity as
 * a relative positive or negative value.
 */
typedef struct Material {
    int quantity;
    char* name; // MALLOC!
} Material;

/* Initialises a new material with the given quantity and material name.
 * A COPY of name is taken and stored as a MALLOC'd string in name.
 */
void mat_init(Material* material, int quantity, char* name);

/* Destroys the material and frees its memory.
 */
void mat_destroy(Material* material);

#endif
