#if !defined(OTTER_DATATYPES_COMMON_H)
#define OTTER_DATATYPES_COMMON_H

#include <stdint.h>

typedef void (*data_destructor_t)(void *);

typedef union {
    void       *ptr;
    uint64_t    value;
} data_item_t;

#endif // OTTER_DATATYPES_COMMON_H
