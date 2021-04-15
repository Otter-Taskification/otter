#if !defined(QUEUE_H)
#define QUEUE_H

// Public

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct queue_t queue_t;

typedef union {
    void       *ptr;
    uint64_t    value;
} queue_data_t;

queue_t       *qu_create(void);
bool           qu_enqueue(queue_t *q, queue_data_t data);
bool           qu_dequeue(queue_t *q, queue_data_t *data);
size_t         qu_get_length(queue_t *q);
bool           qu_is_empty(queue_t *q);
void           qu_destroy(queue_t *q);

#endif // QUEUE_H
