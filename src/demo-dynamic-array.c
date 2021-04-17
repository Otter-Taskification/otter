#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <macros/debug.h>
#include <dtypes/dynamic-array.h>

#define NITEMS 16

int main(void)
{
    size_t length = 0;
    array_t *array = array_create(0);
    assert(array != NULL);
    assert(array_peek_data(array, &length) == NULL);
    assert(array_length(array) == 0);

    array_print(array);

    for (int i=0; i<NITEMS; i++)
    {
        assert(array_push_back(array, (array_element_t){.value=i}));
        array_print(array);
    }

    assert(array_peek_data(array, NULL) == NULL);
    assert(array_peek_data(array, &length) != NULL);

    assert(array_detach_data(NULL, NULL) == NULL);
    assert(array_detach_data(NULL, &length) == NULL);
    free(array_detach_data(array, &length));
    assert(array_length(array) == 0);
    assert(array_peek_data(array, &length) == NULL);

    array_destroy(array);

    assert(array_push_back(NULL, (array_element_t){.value=3}) == false);

    return 0;
}
