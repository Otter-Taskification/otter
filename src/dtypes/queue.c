#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dtypes/queue.h>

#include <macros/debug.h>

typedef struct node_t node_t;

struct node_t {
    queue_data_t    data;
    node_t         *next;
};

struct queue_t {
    node_t      *head;
    node_t      *tail;
    size_t       length;
    destroy_callback_t destroy;
};

queue_t *
qu_create(destroy_callback_t callback)
{
    queue_t *q = malloc(sizeof(*q));
    if (q == NULL)
    {
        LOG_ERROR("failed to create queue");
        return NULL;
    }
    LOG_DEBUG("queue created: %p", q);
    q->head = q->tail = NULL;
    q->length = 0;
    LOG_DEBUG_IF(callback == NULL,
        "queue item destructor not provided, using free()");
    q->destroy = callback == NULL ? &free : callback;
    return q;
}

bool           
qu_enqueue(queue_t *q, queue_data_t data)
{
    if (q == NULL)
    {
        LOG_WARN("queue is null, can't enqueue item");
        return false;
    }

    node_t *node = malloc(sizeof(*node));

    if (node == NULL)
    {
        LOG_ERROR("queue node creation failed for queue %p", q);
        return false;
    }

    node->data = data;
    node->next = NULL;

    if (q->length == 0)
    {
        q->head = q->tail = node;
        q->length = 1;
    } else {
        q->tail->next = node;
        q->tail = node;
        q->length += 1;
    }

    LOG_DEBUG("enqueued item (queue %p: tail=%p, length=%lu)", q, q->tail, q->length);
    return true;
}

bool   
qu_dequeue(queue_t *q, queue_data_t *data)
{
    if (q == NULL)
    {
        LOG_WARN("queue is null, can't dequeue item");
        return false;
    }

    if (q->head == NULL)
    {
        LOG_DEBUG("empty queue");
        return false;
    }

    if (data != NULL) *data = q->head->data;
    LOG_DEBUG_IF(data == NULL,
        "dequeing item with null data pointer, data not written");
    node_t *node = q->head;
    q->head = q->head->next;
    q->length -= 1;
    LOG_DEBUG("dequeued item (queue %p: head=%p, length=%lu) - destroying node",    
        q, q->head, q->length);
    free(node);

    return true;
}

size_t         
qu_get_length(queue_t *q)
{
    return (q == NULL) ? 0 : q->length;
}

bool           
qu_is_empty(queue_t *q)
{
    return (q == NULL) ? true : ((q->length == 0) ? true : false) ;
}

void           
qu_destroy(queue_t *q, bool nodes, bool data)
{
    if (q == NULL) return;
    if (data)
    {
        LOG_DEBUG("destroying queue %p%s", q, ", nodes & data");
        queue_data_t d;
        while(qu_dequeue(q, &d))
        {
            LOG_DEBUG("destroying node data %p", d.ptr);
            q->destroy(d.ptr);
        }
    } else if (nodes) {
        LOG_DEBUG("destroying queue %p%s", q, " & nodes");
        while(qu_dequeue(q, NULL));
    } else {
        LOG_DEBUG("destroying queue %p%s", q, " only");
        LOG_WARN_IF(((q->length != 0)),
            "destroying queue %p (len=%lu) without destroying nodes or data may cause "
            "memory leak", q, q->length);
    }
    free(q);
    return;
}

