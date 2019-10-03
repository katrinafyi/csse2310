#ifndef MATERIAL_H
#define MATERIAL_H

/* Material struct. Can also be used as a delta by specifying quantity as
 * a relative positive or negative value.
 */
typedef struct Material {
    int quantity;
    char* name;
} Material;

#endif
