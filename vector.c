
#include "vector.h"
#include "assert.h"

vector_t * create_vector(size_t init_capacity)
{
    if(init_capacity < 2)
	init_capacity = 2;

    vector_t * v = (vector_t *)malloc(sizeof(vector_t));

    c_assert2(v, "malloc failed");

    v->size = 0;
    v->capacity = init_capacity;
    v->table = (vector_elt_t *)malloc(v->capacity * sizeof(vector_elt_t));

    c_assert2(v->table, "malloc failed");

    return v;
}

void vector_set_element_at(vector_t * v, size_t index, vector_elt_t e)
{
    c_assert(v);
    c_assert2(index < v->size, "index out of bound");

    v->table[index] = e;
}

vector_elt_t vector_get_element_at(vector_t * v, size_t index)
{
    c_assert(v);
    c_assert2(index < v->size, "index out of bound");

    return v->table[index];
}

void vector_add_element(vector_t * v, vector_elt_t e)
{
    c_assert(v);

    if(v->size == v->capacity)
    {
	v->capacity *= 2;
	v->table = realloc(v->table, v->capacity * sizeof(vector_elt_t));

	c_assert2(v->table, "realloc failed");
    }

    v->table[v->size] = e;

    v->size++;
}

void vector_add_element_first(vector_t * v, vector_elt_t e)
{
    vector_add_element(v, e);

    unsigned int i;
    for(i = v->size - 1; i > 0; --i)
	v->table[i] = v->table[i - 1];

    v->table[0] = e;
}

size_t vector_size(vector_t * v)
{
    c_assert(v);
    return v->size;
}

size_t vector_capacity(vector_t * v)
{
    c_assert(v);
    return v->capacity;
}

void free_vector(vector_t * v, int free_elt)
{
    if(!v) return;

    if(free_elt)
    {
	unsigned int i;
	for(i = 0; i < v->size; ++i)
	    free(v->table[i]);
    }

    free(v->table);

    free(v);
}

