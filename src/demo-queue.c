#include <stdbool.h>
#include <assert.h>

#include <dtypes/queue.h>

#define NITEMS 5

int main(void)
{
    queue_data_t data = {.value = 0};
    queue_t *q = qu_create();
    assert(q != NULL);

    for (int i=0; i< NITEMS; i++)
    {
        assert(qu_enqueue(q, (queue_data_t){.value=i}));
        assert(qu_get_length(q) == i+1);
    }
    assert(qu_get_length(q) == 5);
    
    while(qu_dequeue(q, NULL));
    assert(qu_dequeue(q, NULL) == false);

    qu_destroy(q);

    return 0;
}
