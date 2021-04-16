#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <dtypes/queue.h>

#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 2
#endif
#include <macros/debug.h>

#define NITEMS 5

void data_destructor(void *data)
{
    LOG_INFO("destroyed data %p", data);
    return;
}

bool check_create_queue(queue_t **q)
{
    assert(NULL != (*q = qu_create(NULL)));
    return true;
}

bool check_create_queue_with_destructor(queue_t **q, destroy_callback_t c)
{
    assert(NULL != (*q = qu_create(c)));
    return true;
}

bool check_enqueue_is_true(queue_t *q, int items)
{
    for (int i=0; i< items; i++)
    {
        assert(qu_enqueue(q, (queue_data_t){.value=i}));
        assert(qu_get_length(q) == i+1);
    }
    return true;
}

bool check_enqueue_ptr_is_true(queue_t *q, int items)
{
    for (int i=0; i< items; i++)
    {
        assert(qu_enqueue(q, (queue_data_t){.ptr=malloc(1)}));
        assert(qu_get_length(q) == i+1);
    }
    return true;
}

bool check_dequeue_all_is_true(queue_t *q)
{
    while(qu_dequeue(q, NULL));
    assert(qu_dequeue(q, NULL) == false);
    return true;
}

bool check_dequeue_empty_queue_is_false(queue_t *q)
{
    while(qu_dequeue(q, NULL));
    assert(qu_dequeue(q, NULL) == false);
    return true;
}

bool check_dequeue_null_queue_is_false(void)
{
    assert(qu_dequeue(NULL, NULL) == false);
    return true;
}

bool check_null_queue_is_empty_is_true()
{
    assert(qu_is_empty(NULL) == true);
    return true;
}

bool check_null_queue_length_is_0()
{
    assert(qu_get_length(NULL) == 0);
    return true;
}

int main(void)
{
    queue_t *q;

    LOG_INFO(">>> check queueing, dequeueing all and then trying to dequeue "
            "again returns false");
    assert(check_create_queue(&q));
    assert(check_enqueue_is_true(q, NITEMS));
    assert(check_dequeue_all_is_true(q));
    assert(qu_is_empty(q) == true);
    assert(check_dequeue_empty_queue_is_false(q));
    assert(check_dequeue_null_queue_is_false());

    /* check re-queing then destroying nodes but not data */
    LOG_INFO(">>> check re-queing then destroying nodes but not data");
    assert(check_enqueue_is_true(q, NITEMS));
    qu_destroy(q, true, false);
    q = NULL;
    
    LOG_INFO(">>> check creating, queueing with pointers, then destroying "
            "data (should imply destroying nodes");
    assert(check_create_queue(&q));
    assert(check_enqueue_ptr_is_true(q, NITEMS));
    qu_destroy(q, false, true);
    q = NULL;

    LOG_INFO(">>> check creating queue with callback for destroying data");
    assert(check_create_queue_with_destructor(&q, data_destructor));
    assert(check_enqueue_ptr_is_true(q, NITEMS));
    qu_destroy(q, false, true);
    q = NULL;

    LOG_INFO(">>> check null queue is empty and has length 0");
    assert(check_null_queue_is_empty_is_true());
    assert(check_null_queue_length_is_0());

    return 0;
}
