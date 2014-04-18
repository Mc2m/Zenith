
#ifndef __included_umbra_array_h
#define __included_umbra_array_h

/** 
  * Dynamic Array
  */

typedef struct _Array {
	void **data;
	size_t size,used;
	size_t lastindex;
} Array;

Array *array_new(size_t size);

Array *array_copy(const Array *arr, void *(*data_cpy)(void *));
void array_copy_inplace(Array *to, const Array *arr, void *(*data_cpy)(void *));

void array_init(Array *arr,size_t size);

void array_free(Array *arr,void (*free_func)(void *));

void array_empty(Array *arr,void (*free_func)(void *));
void array_empty_full(Array *arr,void (*free_func)(void *));

size_t array_append(Array *arr,void *data);
void array_insert(Array *arr,void *data, size_t index);

void *array_foreach(Array *arr, void *(*func)(void *,void *),void *user_data);
__inline void *array_get(Array *arr,size_t index)
{
	if(index >= arr->lastindex) return 0;
	return arr->data[index];
}

void *array_pop(Array *arr,size_t index);

void array_sort(Array *arr);

void array_remove(Array *arr,size_t index);
void array_remove_fast(Array *arr,size_t index); // grab end element and put it in place
void array_remove_clear(Array *arr,size_t index); // replace with 0

#define array_push array_append

#endif
