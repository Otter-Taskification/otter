#include <otf2/otf2.h>
#include "public/types/stack.h"
#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-region-attr.h"
#include "src/otter-trace/trace-region-types.h"

/* Store values needed to register region definition (tasks, parallel regions, 
   workshare constructs etc.) with OTF2 */
typedef struct {
    OTF2_RegionRef       ref;
    OTF2_RegionRole      role;
    OTF2_AttributeList  *attributes;
    trace_region_type_t  type;
    unique_id_t          encountering_task_id;
    otter_stack_t       *rgn_stack;
    trace_region_attr_t  attr;    
} trace_region_def_t;
