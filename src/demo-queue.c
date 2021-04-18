#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 3
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <dtypes/queue.h>
#include <macros/debug.h>

#define NITEMS 5

void data_destructor(void *data)
{
    LOG_INFO("destroyed data %p", data);
    free(data);
    return;
}

bool check_create_queue(queue_t **q)
{
    assert(NULL != (*q = queue_create(NULL)));
    return true;
}

bool check_create_queue_with_destructor(queue_t **q, data_destructor_t c)
{
    assert(NULL != (*q = queue_create(c)));
    return true;
}

bool check_enqueue_is_true(queue_t *q, int items)
{
    for (int i=0; i< items; i++)
    {
        assert(queue_push(q, (queue_item_t){.value=i}));
        assert(queue_length(q) == i+1);
    }
    return true;
}

bool check_enqueue_ptr_is_true(queue_t *q, int items)
{
    for (int i=0; i< items; i++)
    {
        assert(queue_push(q, (queue_item_t){.ptr=malloc(1)}));
        assert(queue_length(q) == i+1);
    }
    return true;
}

bool check_dequeue_all_is_true(queue_t *q)
{
    while(queue_pop(q, NULL));
    assert(queue_pop(q, NULL) == false);
    return true;
}

bool check_dequeue_empty_queue_is_false(queue_t *q)
{
    while(queue_pop(q, NULL));
    assert(queue_pop(q, NULL) == false);
    return true;
}

bool check_dequeue_null_queue_is_false(void)
{
    assert(queue_pop(NULL, NULL) == false);
    return true;
}

bool check_null_queue_is_empty_is_true()
{
    assert(queue_is_empty(NULL) == true);
    return true;
}

bool check_null_queue_length_is_0()
{
    assert(queue_length(NULL) == 0);
    return true;
}

bool check_queue_is_empty(queue_t *q)
{
    assert(queue_length(q) == 0);
    return true;
}

int main(void)
{
    queue_t *q;

    LOG_INFO(">>> check queueing, dequeueing all and then trying to dequeue "
            "again returns false");
    assert(check_create_queue(&q));
    assert(check_enqueue_is_true(q, NITEMS));
    #if DEBUG_LEVEL >= 4
    queue_print(q);
    #endif
    assert(check_dequeue_all_is_true(q));
    assert(queue_is_empty(q) == true);
    assert(check_dequeue_empty_queue_is_false(q));
    assert(check_dequeue_null_queue_is_false());

    /* check re-queing then destroying nodes but not data */
    LOG_INFO(">>> check re-queing then destroying nodes but not data");
    assert(check_enqueue_is_true(q, NITEMS));
    #if DEBUG_LEVEL >= 4
    queue_print(q);
    #endif
    queue_destroy(q, false);
    q = NULL;
    
    LOG_INFO(">>> check creating, queueing with pointers, then destroying "
            "data (should imply destroying nodes");
    assert(check_create_queue(&q));
    assert(check_enqueue_ptr_is_true(q, NITEMS));
    #if DEBUG_LEVEL >= 4
    queue_print(q);
    #endif
    queue_destroy(q, true);
    q = NULL;

    LOG_INFO(">>> check creating queue with callback for destroying data");
    assert(check_create_queue_with_destructor(&q, data_destructor));
    assert(check_enqueue_ptr_is_true(q, NITEMS));
    queue_destroy(q, true);
    q = NULL;

    LOG_INFO(">>> check queue with 1 item correctly emptied");
    assert(check_create_queue(&q));    
    assert(check_enqueue_is_true(q, 1));
    while(queue_pop(q, NULL));
    assert(check_queue_is_empty(q));
    queue_destroy(q, true);
    q = NULL;

    LOG_INFO(">>> check null queue is empty and has length 0");
    assert(check_null_queue_is_empty_is_true());
    assert(check_null_queue_length_is_0());

    return 0;
}
