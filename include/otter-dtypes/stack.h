#if !defined(STACK_H)
#define STACK_H

// Public

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <macros/debug.h>

typedef void (*data_destructor_t)(void *);

typedef struct stack_t stack_t;

typedef union {
    void       *ptr;
    uint64_t    value;
} stack_item_t;

stack_t       *stack_create(data_destructor_t destructor);
bool           stack_push(stack_t *s, stack_item_t item);
bool           stack_pop(stack_t *s, stack_item_t *dest);
bool           stack_peek(stack_t *s, stack_item_t *dest);
size_t         stack_size(stack_t *s);
bool           stack_is_empty(stack_t *s);
void           stack_destroy(stack_t *s, bool items);

#if DEBUG_LEVEL >= 4
void           stack_print(stack_t *s);
#endif

#endif // STACK_H
