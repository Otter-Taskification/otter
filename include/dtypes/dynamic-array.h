#if !defined(DYNAMIC_ARRAY_H)
#define DYNAMIC_ARRAY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct dynamic_array_t dynamic_array_t;

typedef union {
    void       *ptr;
    uint64_t    value;
} array_element_t;

dynamic_array_t   *da_create(size_t length);
bool               da_push_back(dynamic_array_t *arr, array_element_t elem);
size_t             da_get_length(dynamic_array_t *arr);
array_element_t   *da_peek_data(dynamic_array_t *arr, size_t *length);
array_element_t   *da_detach_data(dynamic_array_t *arr, size_t *length);
void               da_destroy(dynamic_array_t *arr);

#endif // DYNAMIC_ARRAY_H
