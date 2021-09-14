#if !defined(OTTER_STACK_H)
#define OTTER_STACK_H

// Public

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "otter/debug.h"
#include "otter/datatypes-common.h"

typedef struct otter_stack_t otter_stack_t;

otter_stack_t *stack_create(void);
bool     stack_push(otter_stack_t *s, data_item_t item);
bool     stack_pop(otter_stack_t *s, data_item_t *dest);
bool     stack_peek(otter_stack_t *s, data_item_t *dest);
size_t   stack_size(otter_stack_t *s);
bool     stack_is_empty(otter_stack_t *s);
void     stack_destroy(otter_stack_t *s, bool items, data_destructor_t destructor);

#if DEBUG_LEVEL >= 4
void           stack_print(otter_stack_t *s);
#endif

#ifdef __cplusplus
}
#endif

#endif // OTTER_STACK_H
