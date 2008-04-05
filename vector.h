#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

/** Structure de vecteur generique
 */

typedef void* vector_elt_t;

typedef struct
{
    vector_elt_t * table;
    size_t size;
    size_t capacity;
} vector_t;


vector_t * create_vector(size_t init_capacity);

void vector_set_element_at(vector_t * v, size_t index, vector_elt_t e);
vector_elt_t vector_get_element_at(vector_t * v, size_t index);
void vector_add_element(vector_t * v, vector_elt_t e);

void vector_add_element_first(vector_t * v, vector_elt_t e);

size_t vector_size(vector_t * v);
size_t vector_capacity(vector_t * v);


void clear_vector(vector_t * v, int free_elt);

/** Detruit le vecteur v, si free_elt est non nul
 * les elements du vecteur sont detruits aussi.
 */
void free_vector(vector_t * v, int free_elt);



#define BOX_UINT(i) ((vector_elt_t)(i))
#define UNBOX_UINT(i) ((unsigned int)(i))

#endif
