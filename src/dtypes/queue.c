#include <stdlib.h>
#include <stdint.h>
#include "dtypes/queue.h"

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
    if (q == NULL) return NULL;
    q->head = q->tail = NULL;
    q->length = 0;
}

bool           
qu_enqueue(queue_t *q, queue_data_t data)
{
    return false;
}

queue_data_t   
qu_dequeue(queue_t *q)
{
    return (queue_data_t){.value = 0};
}

size_t         
qu_get_length(queue_t *q)
{
    return 0;
}

bool           
qu_is_empty(queue_t *q)
{
    return true;
}

void           
qu_destroy(queue_t *q)
{
    return;
}

