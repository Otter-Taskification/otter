#if !defined(OTTER_QUEUE_H)
#define OTTER_QUEUE_H

// Public

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <macros/debug.h>
#include <otter-datatypes/datatypes-common.h>

typedef struct queue_t queue_t;

queue_t *queue_create(void);
bool     queue_push(queue_t *q, data_item_t item);
bool     queue_pop(queue_t *q, data_item_t *dest);
size_t   queue_length(queue_t *q);
bool     queue_is_empty(queue_t *q);
void     queue_destroy(queue_t *q, bool items, data_destructor_t destructor);

/* append the items in r to q */
bool queue_append(queue_t *q, queue_t *r);

/* scan through the items in a queue without modifying the queue
   write the current queue item to dest
*/
void queue_scan(queue_t *q, data_item_t *dest, void **next);

#if DEBUG_LEVEL >= 4
void           queue_print(queue_t *q);
#endif

#endif // OTTER_QUEUE_H
