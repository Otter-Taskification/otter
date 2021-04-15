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
};

queue_t *
qu_create(void)
{
    queue_t *q = malloc(sizeof(*q));
    if (q == NULL)
    {
        LOG_ERROR("failed to create queue");
        return NULL;
    }
    LOG_DEBUG("created queue: %p", q);
    q->head = q->tail = NULL;
    q->length = 0;
    return q;
}

bool           
qu_enqueue(queue_t *q, queue_data_t data)
{
    if (q == NULL)
    {
        LOG_DEBUG("tried to enqueue with null queue");
        return false;
    }

    node_t *node = malloc(sizeof(*node));

    if (node == NULL)
    {
        LOG_ERROR("failed to create node for queue %p", q);
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

    LOG_DEBUG("tail of %p is now %p (length=%lu)", q, q->tail, q->length);
    return true;
}

bool   
qu_dequeue(queue_t *q, queue_data_t *data)
{
    if (q == NULL)
    {
        LOG_DEBUG("tried to dequeue from null queue");
        return false;
    }

    if (q->head == NULL)
    {
        LOG_DEBUG("empty queue");
        return false;
    }

    if (data != NULL) *data = q->head->data;
    LOG_DEBUG_IF(data == NULL, "null pointer passed as data argument");
    node_t *node = q->head;
    q->head = q->head->next;
    q->length -= 1;
    LOG_DEBUG("head of %p is now %p (length=%lu)", q, q->head, q->length);

    LOG_DEBUG("freeing node %p", node);
    free(node);

    return (q->head == NULL) ? false : true ;
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
qu_destroy(queue_t *q)
{
    LOG_DEBUG("destroying queue: %p", q);
    if (q == NULL) return;
    LOG_WARN_IF(q->length != 0,
        "destroying non-empty queue %p may cause memory leak", q);
    free(q);
    return;
}

