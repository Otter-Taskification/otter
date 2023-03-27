#if !defined(OTTER_TRACE_ATTRIBUTES_H)
#define OTTER_TRACE_ATTRIBUTES_H

/* Enum values are used as an index to lookup string refs for a given attribute
   name & label */
typedef enum {
#define INCLUDE_ATTRIBUTE(Type, Name, Desc) attr_##Name,
#include "src/otter-trace/trace-attribute-defs.h"
    n_attr_defined
} attr_name_enum_t;

typedef enum {
#define INCLUDE_LABEL(Name, Label) attr_##Name##_##Label,
#include "src/otter-trace/trace-attribute-defs.h"
    n_attr_label_defined
} attr_label_enum_t;

#endif // OTTER_TRACE_ATTRIBUTES_H
