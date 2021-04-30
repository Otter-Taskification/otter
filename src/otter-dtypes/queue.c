#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <otter-dtypes/queue.h>
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
    LOG_DEBUG("%p", q);
    q->head = q->tail = NULL;
    q->length = 0;
    LOG_DEBUG_IF(destructor == NULL,
        "%p: destructor is free()", q);
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

    LOG_DEBUG("%p[%lu]=%p", q, q->length-1, item.ptr);

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
        LOG_DEBUG("%p[0]=%p", q, q->head);
        return false;
    }

    if (dest != NULL) *dest = q->head->data;
    node_t *node = q->head;
    q->head = q->head->next;
    q->length -= 1;
    LOG_DEBUG("%p[0] -> %p", q, dest->ptr);
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

    LOG_WARN_IF(((q->length != 0) && (items == false)),
        "destroying queue %p (len=%lu) without destroying items may cause "
        "memory leak", q, q->length);

    queue_item_t d = {.value = 0};
    while(queue_pop(q, &d))
    {
        LOG_DEBUG("%p[0/%lu]=%p", q, q->length-1, d.ptr);
        if (items) q->destroy(d.ptr);
    }
    LOG_DEBUG("%p", q);
    free(q);
    return;
}

/* transfer items from r to q */
bool
queue_append(
    queue_t *q,
    queue_t *r)
{
    if ((q == NULL) || (r == NULL)) return false;

    if (r->length == 0) return true;

    #if DEBUG_LEVEL >= 4
    queue_print(q);
    queue_print(r);
    #endif

    if (q->length == 0)
        q->head   = r->head;
    else
        q->tail->next = r->head;
    
    q->tail   = r->tail;
    q->length = q->length + r->length;
    r->head   = r->tail = NULL;
    r->length = 0;

    #if DEBUG_LEVEL >= 4
    queue_print(q);
    queue_print(r);
    #endif

    return true;
}

/* scan through the items in a queue without modifying the queue
   write the current queue item to dest
   save the address of the next item in the queue to [next]
   if [next] == NULL, start with queue->head
   (NOTE: up to the caller to track how many items to scan, otherwise will loop)
*/
void
queue_scan(
    queue_t *q,
    queue_item_t *dest,
    void **next)
{
    if ((next == NULL) || (dest == NULL))
    {
        LOG_ERROR("null pointer");
        return;
    }

    node_t *next_node = (node_t*) *next;

    if (next_node == NULL)
    {
        *dest = q->head->data;
        *next = (void*) q->head->next;
    } else {
        *dest = next_node->data;
        *next = (void*) next_node->next;
    }
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
