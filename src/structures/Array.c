
#include "stdafx.h"

#include "Array.h"

#define AppAssert assert

static __inline size_t upper_power_of_two(size_t v)
{
	v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

void array_grow(Array *arr,size_t minsize)
{
	if(!arr->size) {
		arr->data = (void **) malloc(sizeof(void *) * minsize);
		AppAssert(arr->data);

		arr->size = minsize;
		memset(arr->data,0,sizeof(void *) * minsize);
	} else {
		size_t oldsize = arr->size;
		arr->size = upper_power_of_two(minsize);

		arr->data = (void **) realloc(arr->data,sizeof(void *) * arr->size);
		AppAssert(arr->data);

		memset(arr->data+oldsize,0,sizeof(void *) *(arr->size-oldsize));
	}
}

void array_shrink(Array *arr)
{
	if(arr->lastindex >= 1) {
		arr->size = upper_power_of_two(arr->lastindex);

		arr->data = (void **) realloc(arr->data,sizeof(void *) * arr->size);
		AppAssert(arr->data);
	}
}

Array *array_new(size_t size)
{
	Array *arr = (Array *) malloc(sizeof(Array));
	AppAssert(arr);

	array_init(arr,size);

	return arr;
}

Array *array_copy(const Array *arr, void *(*data_cpy)(void *))
{
	Array *cpy;
	size_t i = 0;
	
	if(!arr) return 0;

	cpy = array_new(arr->size);
	
	cpy->used = arr->used;
	cpy->lastindex = arr->lastindex;

	for(; i < arr->lastindex; ++i) cpy->data[i] = data_cpy ? data_cpy(arr->data[i]) : arr->data[i];

	return cpy;
}

void array_copy_inplace(Array *to, const Array *arr, void *(*data_cpy)(void *))
{
	size_t i = 0;
	if(!to || !arr) return;

	if(to->size < arr->size) array_grow(to,arr->lastindex);

	to->used = arr->used;
	to->lastindex = arr->lastindex;

	for(; i < arr->lastindex; ++i) to->data[i] = data_cpy ? data_cpy(arr->data[i]) : arr->data[i];
}

void array_init(Array *arr,size_t size)
{
	if(size) {
		arr->data = (void **) malloc(sizeof(void *) * size);
		AppAssert(arr->data);

		memset(arr->data,0,size);
	} else {
		arr->data = 0;
	}

	arr->size = size;
	arr->used = 0;
	arr->lastindex = 0;
}

void array_free(Array *arr,void (*free_func)(void *))
{
	if(arr) {
		if(free_func) {
			size_t i = 0;
			for(; i < arr->size && arr->used; ++i) {
				if(arr->data[i]) {
					arr->used--;
					free_func(arr->data[i]);
					arr->data[i] = 0;
				}
			}
		}

		free(arr->data);
		free(arr);
	}
}

void array_empty(Array *arr,void (*free_func)(void *))
{
	size_t i = 0;
	for(; i < arr->lastindex && arr->used; ++i) {
		if(arr->data[i]) {
			arr->used--;
			free_func(arr->data[i]);
			arr->data[i] = 0;
		}
	}
	arr->lastindex = 0;
}

void array_empty_full(Array *arr,void (*free_func)(void *))
{
	array_empty(arr,free_func);
	free(arr->data);
	arr->data = 0;
	arr->size = 0;
}

size_t array_append(Array *arr,void *data)
{
	array_insert(arr,data,arr->lastindex);
	return arr->lastindex-1;
}

void array_insert(Array *arr,void *data, size_t index)
{
	if(index >= arr->size) array_grow(arr,index+1);
	if(!arr->data[index]) arr->used += 1;
	arr->data[index] = data;
	arr->lastindex = max(index+1,arr->lastindex);
}

void *array_foreach(Array *arr, void *(*func)(void *,void *),void *user_data)
{
	size_t i = 0;
	for(; i < arr->lastindex; ++i) {
		void *value = func(arr->data[i],user_data);
		if(value) return value;
	}

	return 0;
}

void *array_pop(Array *arr,size_t index) {
	void *data;

	if(index >= arr->lastindex) return 0;

	data = arr->data[index];
	if(data) arr->used -= 1;

	for(; index < arr->lastindex-1; ++index) arr->data[index] = arr->data[index+1];
	arr->lastindex -= 1;

	if(arr->lastindex <= (arr->size >> 2)) array_shrink(arr);

	return data;
}

void array_sort(Array *arr)
{
	//TODO
}

void array_remove(Array *arr,size_t index)
{
	if(index >= arr->lastindex) return;

	if(arr->data[index]) arr->used -= 1;

	for(; index < arr->lastindex-1; ++index) arr->data[index] = arr->data[index+1];
	arr->lastindex -= 1;

	if(arr->lastindex <= (arr->size >> 2)) array_shrink(arr);
}

void array_remove_fast(Array *arr,size_t index)
{
	if(index >= arr->lastindex) return;

	if(arr->data[index]) arr->used -= 1;

	arr->data[index] = arr->data[arr->lastindex-1];
	arr->data[arr->lastindex-1] = 0;
	arr->lastindex -= 1;

	if(arr->lastindex <= (arr->size >> 2)) array_shrink(arr);
}

void array_remove_clear(Array *arr,size_t index)
{
	if(index >= arr->lastindex) return;

	if(arr->data[index]) arr->used -= 1;

	arr->data[index] = 0;
}
