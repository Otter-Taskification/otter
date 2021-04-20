#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <otter-dt/queue.h>
#include <macros/debug.h>

typedef struct node_t node_t;

struct node_t {
    queue_item_t    data;
    node_t         *next;
};

struct queue_t {
    node_t      *head;
    node_t      *tail;
    size_t       length;
    data_destructor_t destroy;
};

queue_t *
queue_create(data_destructor_t destructor)
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
    LOG_DEBUG_IF(destructor == NULL,
        "queue item destructor not provided, using free()");
    q->destroy = destructor == NULL ? &free : destructor;
    return q;
}

bool           
queue_push(queue_t *q, queue_item_t item)
{
    if (q == NULL)
    {
        LOG_WARN("queue is null, can't add item");
        return false;
    }

    node_t *node = malloc(sizeof(*node));

    if (node == NULL)
    {
        LOG_ERROR("queue node creation failed for queue %p", q);
        return false;
    }

    node->data = item;
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

    LOG_DEBUG("queue added item 0x%lx (queue %p: tail node=%p, length=%lu)",
        item.value, q, q->tail, q->length);
    return true;
}

bool   
queue_pop(queue_t *q, queue_item_t *dest)
{
    if (q == NULL)
    {
        LOG_WARN("queue is null");
        return false;
    }

    if (q->head == NULL)
    {
        LOG_DEBUG("queue empty (queue: %p)", q);
        return false;
    }

    if (dest != NULL) *dest = q->head->data;
    node_t *node = q->head;
    q->head = q->head->next;
    q->length -= 1;
    LOG_DEBUG("queue popped item 0x%lx (queue %p: head node=%p, length=%lu)",
        node->data.value, q, q->head, q->length);
    LOG_WARN_IF(dest == NULL,
        "queue popped item without returning value (null destination pointer)");
    free(node);

    return true;
}

size_t         
queue_length(queue_t *q)
{
    return (q == NULL) ? 0 : q->length;
}

bool           
queue_is_empty(queue_t *q)
{
    return (q == NULL) ? true : ((q->length == 0) ? true : false) ;
}

void           
queue_destroy(queue_t *q, bool items)
{
    if (q == NULL) return;
    if (items)
    {
        LOG_DEBUG("destroying queue %p%s", q, " and items");
        queue_item_t d = {.value = 0};
        while(queue_pop(q, &d))
        {
            LOG_DEBUG("destroying queue item %p", d.ptr);
            q->destroy(d.ptr);
        }
    } else {
        LOG_DEBUG("destroying queue %p", q);
        LOG_WARN_IF(((q->length != 0)),
            "destroying queue %p (len=%lu) without destroying items may cause "
            "memory leak", q, q->length);
    }
    free(q);
    return;
}

#if DEBUG_LEVEL >= 4
void
queue_print(queue_t *q)
{
    if (q == NULL)
    {
        fprintf(stderr, "\n%12s\n", "<null queue>");
        return;
    }

    node_t *node = q->head;

    fprintf(stderr, "\n"
                    "%12s %p\n"
                    "%12s %p\n"
                    "%12s %p\n"
                    "%12s %lu\n"
                    "%12s %p\n\n",
                    "QUEUE",        q,
                    "head node",    q->head,
                    "tail node",    q->tail,
                    "length",       q->length,
                    "destructor",   q->destroy);

    const char *sep = " | ";
    fprintf(stderr, "%12s%s%-12s%s%-8s\n", "position", sep, "node", sep, "item");
    int position = 0;
    while (node != NULL)
    {
        fprintf(stderr, "%12d%s%-12p%s0x%06lx (%lu)\n", position, sep, node, sep, node->data.value, node->data.value);
        node = node->next;
        position++;
    }
    fprintf(stderr, "\n");
    return;
}
#endif
