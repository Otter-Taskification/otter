#include <stdlib.h>
#include <dtypes/dynamic-array.h>
#include <macros/debug.h>

#if !defined(DA_LEN)
#define DEFAULT_LENGTH 1000
#else
#define DEFAULT_LENGTH DA_LEN
#endif

#if !defined(DA_INC)
#define DEFAULT_INCREMENT 1000
#else
#define DEFAULT_INCREMENT DA_INC
#endif

// Private

array_element_t *da_extend(dynamic_array_t *array, size_t new_length);

struct dynamic_array_t {
    array_element_t *begin;
    array_element_t *end;
    array_element_t *tail;
};

dynamic_array_t *
da_create(size_t length)
{
    dynamic_array_t *array = malloc(sizeof(*array));
    if (array == NULL) 
        {LOG_ERROR("failed to create dynamic array"); return NULL;}
    if (length == 0) {
        array->begin = array->tail = array->end = NULL;
        return array;
    }
    array->begin = malloc(length * sizeof(array_element_t));
    if (array->begin == NULL) {
        LOG_ERROR("failed to allocate dynamic array block of size %lu",
            length*sizeof(array_element_t));
        free(array);
        return NULL;
    }    
    array->tail = array->begin + (length * sizeof(array_element_t));
    array->end = array->begin;
    LOG_DEBUG("created dynamic array %p (len=%lu)", array, length);
    return array;
}

bool             
da_push_back(dynamic_array_t *array, array_element_t elem)
{
    if (array == NULL) {LOG_ERROR("null array pointer"); return NULL;}
    LOG_DEBUG("appending value %p to array %p", elem.ptr,  array);
    if (array->end >= array->tail)
    {
        size_t length = da_get_length(array);
        void *new_data = da_extend(array, length + DEFAULT_INCREMENT);
        if (new_data == NULL) return false;
    }
    *(array->end) = elem;
    array->end = array->end + sizeof(array_element_t);
    return true;
}

size_t           
da_get_length(dynamic_array_t *array)
{
    if (array == NULL) return 0;
    LOG_DEBUG("arr=%p begin=%p end=%p diff=%lu)",
        array, array->begin, array->end, array->end - array->begin);
    return (array == NULL) ? 0 : 
        (array->end - array->begin)/sizeof(array_element_t);
}

array_element_t *
da_peek_data(dynamic_array_t *array, size_t *length)
{
    if ((array == NULL) || (length == NULL))
    {
        LOG_ERROR("null pointer (array=%p, length=%p)", array, length);
        return NULL;
    } else {
        *length = da_get_length(array);
        LOG_DEBUG("return data %p (len=%lu) from array %p",
            array->begin, da_get_length(array), array);
        return array->begin;
    }
}

array_element_t *
da_detach_data(dynamic_array_t *array, size_t *length)
{
    LOG_DEBUG("detaching data from array %p", array);
    if ((array == NULL) || (length == NULL))
    {
        LOG_ERROR("null pointer (array=%p, length=%p)", array, length);
        return NULL;
    }
    array_element_t *data = da_peek_data(array, length);
    if (data == NULL) return NULL;
    array->begin = array->end = array->tail = NULL;
    LOG_DEBUG("detached data from array %p", array);
    return data;
}

void             
da_destroy(dynamic_array_t *array)
{
    if (array == NULL) {LOG_ERROR("null array pointer"); return;}
    LOG_DEBUG("destroying dynamic array %p and data %p", array, array->begin);
    if (array->begin != NULL) free(array->begin);
    free(array);
    return;
}

array_element_t *
da_extend(dynamic_array_t *array, size_t new_length)
{
    if (array == NULL) {LOG_ERROR("null array pointer"); return NULL;}
    LOG_DEBUG("extending array %p to %lu elements",
        array, new_length);
    void *new_data = reallocarray(array->begin, new_length, sizeof(array_element_t));
    if (new_data == NULL)
        {LOG_ERROR("failed to extend array %p", array); return NULL; }
    if (new_data != array->begin)
    {
        LOG_DEBUG("data was relocated from %p to %p, updating array",
            array->begin, new_data);
        size_t offset = array->end - array->begin;
        array->begin = new_data;
        array->end = array->begin + offset;
    }
    array->tail = array->begin + (new_length * sizeof(array_element_t));
    return array->begin;
}
