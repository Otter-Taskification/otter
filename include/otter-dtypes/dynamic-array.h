#if !defined(DYNAMIC_ARRAY_H)
#define DYNAMIC_ARRAY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <macros/debug.h>

typedef struct array_t array_t;

typedef union {
    void       *ptr;
    uint64_t    value;
} array_element_t;

array_t           *array_create(size_t length);
bool               array_push_back(array_t *arr, array_element_t elem);
size_t             array_length(array_t *arr);
array_element_t   *array_peek_data(array_t *arr, size_t *length);
array_element_t   *array_detach_data(array_t *arr, size_t *length);
void               array_destroy(array_t *arr);

#if DEBUG_LEVEL >= 4
void               array_print(array_t *arr);
#endif

#endif // DYNAMIC_ARRAY_H
