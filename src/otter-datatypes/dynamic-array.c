#include <stdlib.h>

#include <macros/debug.h>
#include <otter-datatypes/dynamic-array.h>

#if !defined(DA_LEN) || (EXPAND(DA_LEN) == 1)
#define DEFAULT_LENGTH 3000
#else
#define DEFAULT_LENGTH DA_LEN
#endif

#if !defined(DA_INC) || (EXPAND(DA_INC) == 1)
#define DEFAULT_INCREMENT 1000
#else
#define DEFAULT_INCREMENT DA_INC
#endif

array_element_t *array_extend(array_t *array, size_t new_length);

struct array_t {
    array_element_t *begin;
    array_element_t *end;
    array_element_t *tail;
};

array_t *
array_create(size_t length)
{
    array_t *array = malloc(sizeof(*array));
    if (array == NULL)
    {
        LOG_ERROR("failed to create array");
        return NULL;
    }
    if (length == 0) {
        array->begin = array->tail = array->end = NULL;
        return array;
    }
    array->begin = malloc(length * sizeof(array_element_t));
    if (array->begin == NULL) {
        LOG_ERROR("failed to allocate array block of size %lu",
            length*sizeof(array_element_t));
        free(array);
        return NULL;
    }    
    array->tail = array->begin + length;
    array->end = array->begin;
    LOG_DEBUG("%p[%lu]", array, length);
    return array;
}

bool             
array_push_back(array_t *array, array_element_t elem)
{
    void *new_data = NULL;
    if (array == NULL)
    {
        LOG_ERROR("array pointer is null");
        return false;
    }
    if (array->begin == NULL) {
        new_data = array_extend(array, DEFAULT_LENGTH);
        if (new_data == NULL) return false;
    } else if (array->end >= array->tail)
    {
        size_t length = array_length(array);
        new_data = array_extend(array, length + DEFAULT_INCREMENT);
        if (new_data == NULL) return false;
    }
    *(array->end) = elem;
    array->end = array->end + 1;
    LOG_DEBUG("%p[%lu/%lu]=%p", array, array->end - 1 - array->begin, array->tail - array->begin - 1, elem.ptr);
    return true;
}

size_t           
array_length(array_t *array)
{
    if (array == NULL) return 0;
    LOG_DEBUG("%p=%lu/%lu", array, array->end - array->begin, array->tail - array->begin);
    return (array == NULL) ? 0 : 
        (array->end - array->begin);
}

array_element_t *
array_peek_data(array_t *array, size_t *length)
{
    if ((array == NULL) || (length == NULL))
    {
        LOG_WARN("null pointer argument (array=%p, length=%p)", array, length);
        return NULL;
    } else {
        *length = array_length(array);
        LOG_DEBUG("%p->%p:%p",
            array, array->begin, array->end == NULL ? NULL : array->end-1);
        return array->begin;
    }
}

array_element_t *
array_detach_data(array_t *array, size_t *length)
{
    if ((array == NULL) || (length == NULL))
    {
        LOG_WARN("null pointer argument (array=%p, length=%p)", array, length);
        return NULL;
    }
    array_element_t *data = array_peek_data(array, length);
    if (data == NULL) return NULL;
    array->begin = array->end = array->tail = NULL;
    LOG_DEBUG("%p->%p", array, data);
    return data;
}

void             
array_destroy(array_t *array)
{
    if (array == NULL) {LOG_ERROR("null array pointer"); return;}
    LOG_DEBUG("%p->%p", array, array->begin);
    if (array->begin != NULL) free(array->begin);
    free(array);
    return;
}

array_element_t *
array_extend(array_t *array, size_t new_length)
{
    if (array == NULL)
    {
        LOG_ERROR("null array pointer");
        return NULL;
    }
    LOG_DEBUG("%p: %lu -> %lu", array, array->tail - array->begin, new_length);
    void *new_data = realloc(array->begin, new_length*sizeof(*(array->begin)));
    if (new_data == NULL)
    {
        LOG_ERROR("failed to extend array %p", array);
        return NULL;
    }
    if (new_data != array->begin)
    {
        LOG_DEBUG("%p->%p is now %p->%p", array, array->begin, array, new_data);
        size_t offset = array->end - array->begin;
        array->begin = new_data;
        array->end = array->begin + offset;
    }
    array->tail = array->begin + new_length;
    return array->begin;
}

#if DEBUG_LEVEL >= 4
void
array_print(array_t *arr)
{
    size_t items = array_length(arr);
    array_element_t *array = arr->begin;

    fprintf(stderr, "\n" 
            "%12s: %p\n"
            "%12s: %lu\n"
            "%12s: %lu\n"
            "%12s: %p (%lu)\n"
            "%12s: %p (%lu)\n"
            "%12s: %p (%lu)\n\n",
            "ARRAY",    arr,
            "items",    items,
            "size",     items * sizeof(array_element_t),
            "begin",    arr->begin, (uint64_t) arr->begin,
            "end",      arr->end,   (uint64_t) arr->end,
            "tail",     arr->tail,  (uint64_t) arr->tail);

    if (items == 0) return;

    fprintf(stderr, "%12s%s%-10s%s%-6s\n", "index", " | ", "address", " | ", "value");
    for (int i=0; i<items; i++)
    {
        fprintf(stderr, "%12d%s%10p%s0x%06lx\n", i, " | ", &array[i], " | ", array[i].value);
    }
    fprintf(stderr, "\n");

    return;
}
#endif