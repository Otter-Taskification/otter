#if !defined(OTTER_QUEUE_H)
#define OTTER_QUEUE_H

// Public

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "otter/debug.h"
#include "otter/datatypes-common.h"

typedef struct otter_queue_t otter_queue_t;

otter_queue_t *queue_create(void);
bool     queue_push(otter_queue_t *q, data_item_t item);
bool     queue_pop(otter_queue_t *q, data_item_t *dest);
size_t   queue_length(otter_queue_t *q);
bool     queue_is_empty(otter_queue_t *q);
void     queue_destroy(otter_queue_t *q, bool items, data_destructor_t destructor);

/* append the items in r to q */
bool queue_append(otter_queue_t *q, otter_queue_t *r);

/* scan through the items in a queue without modifying the queue
   write the current queue item to dest
*/
// void queue_scan(otter_queue_t *q, data_item_t *dest, void **next);

#if DEBUG_LEVEL >= 4
void           queue_print(otter_queue_t *q);
#endif

#ifdef __cplusplus
}
#endif

#endif // OTTER_QUEUE_H
