#include "trace-unique-refs.h"
#include <stdint.h>

/* Different kinds of unique IDs */
typedef enum {
  trace_region,
  trace_string,
  trace_location,
  trace_other,
  NUM_REF_TYPES // NOTE: must be last enum label
} trace_ref_type_t;

static uint64_t get_unique_uint64_ref(trace_ref_type_t ref_type) {
  static uint64_t id[NUM_REF_TYPES] = {0};
  return __sync_fetch_and_add(&id[ref_type], 1L);
}

static uint32_t get_unique_uint32_ref(trace_ref_type_t ref_type) {
  static uint32_t id[NUM_REF_TYPES] = {0};
  return __sync_fetch_and_add(&id[ref_type], 1);
}

OTF2_RegionRef get_unique_rgn_ref(void) {
  return (OTF2_RegionRef)get_unique_uint32_ref(trace_region);
}

OTF2_StringRef get_unique_str_ref(void) {
  return (OTF2_StringRef)get_unique_uint32_ref(trace_string);
}

OTF2_LocationRef get_unique_loc_ref(void) {
  return (OTF2_LocationRef)get_unique_uint64_ref(trace_location);
}
