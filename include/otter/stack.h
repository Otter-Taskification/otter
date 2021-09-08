#if !defined(OTTER_STACK_H)
#define OTTER_STACK_H

// Public

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "otter/debug.h"
#include "otter/datatypes-common.h"

typedef struct stack_t stack_t;

stack_t *stack_create(void);
bool     stack_push(stack_t *s, data_item_t item);
bool     stack_pop(stack_t *s, data_item_t *dest);
bool     stack_peek(stack_t *s, data_item_t *dest);
size_t   stack_size(stack_t *s);
bool     stack_is_empty(stack_t *s);
void     stack_destroy(stack_t *s, bool items, data_destructor_t destructor);

#if DEBUG_LEVEL >= 4
void           stack_print(stack_t *s);
#endif

#endif // OTTER_STACK_H
