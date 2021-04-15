#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <macros/debug.h>
#include <dtypes/dynamic-array.h>

#define NITEMS 6

int main(void)
{
    size_t length = 0;
    dynamic_array_t *array = da_create(0);
    assert(array != NULL);
    assert(da_peek_data(array, &length) == NULL);
    assert(da_get_length(array) == 0);

    for (int i=0; i<NITEMS; i++)
    {
        assert(da_push_back(array, (array_element_t){.value=i}));
    }
    assert(da_get_length(array) == NITEMS);

    assert(da_peek_data(array, NULL) == NULL);
    assert(da_peek_data(array, &length) != NULL);

    assert(da_detach_data(NULL, NULL) == NULL);
    assert(da_detach_data(NULL, &length) == NULL);
    free(da_detach_data(array, &length));
    assert(da_get_length(array) == 0);
    assert(da_peek_data(array, &length) == NULL);

    da_destroy(array);

    assert(da_push_back(NULL, (array_element_t){.value=3}) == false);

    return 0;
}
