
#include "vector.h"
#include "assert.h"

int vector_internal_partition(vector_elt_t * a, int l, int r, sort_fn_t fn);
void vector_internal_quicksort(vector_elt_t * a, int l, int r, sort_fn_t fn);


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

void vector_sort(vector_t * v, sort_fn_t fn)
{
    c_assert(v && fn);

    if(v->size == 0)
	return;

    vector_internal_quicksort(v->table, 0, v->size - 1, fn);
}

void clear_vector(vector_t * v, int free_elt)
{
    c_assert(v);
    if(free_elt)
    {
	unsigned int i;
	for(i = 0; i < v->size; ++i)
	    free(v->table[i]);
    }
    v->size = 0;
}

void free_vector(vector_t * v, int free_elt)
{
    if(!v) return;

    clear_vector(v, free_elt);

    free(v->table);

    free(v);
}





void vector_internal_quicksort(vector_elt_t * a, int l, int r, sort_fn_t fn)
{
   int j;
   if(l < r)
   {
       j = vector_internal_partition(a, l, r, fn);
       vector_internal_quicksort(a, l, j - 1, fn);
       vector_internal_quicksort(a, j + 1, r, fn);
   }
}


int vector_internal_partition(vector_elt_t * a, int l, int r, sort_fn_t fn)
{
   int i, j;
   vector_elt_t t, pivot = a[l];
   i = l; j = r + 1;

   while(1)
   {
        do ++i; while(i <= r && (*fn)(a[i], pivot) <= 0);
   	do --j; while((*fn)(a[j], pivot) > 0);
   	if( i >= j ) break;
   	t = a[i]; a[i] = a[j]; a[j] = t;
   }
   t = a[l]; a[l] = a[j]; a[j] = t;
   return j;
}


