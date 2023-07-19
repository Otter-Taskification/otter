#if !defined(OTTER_TRACE_UNIQUE_REFS_H)
#define OTTER_TRACE_UNIQUE_REFS_H

#include <otf2/OTF2_GeneralDefinitions.h>

// Implement unique references for various OTF2 constructs
// Internal to otter-trace only

OTF2_RegionRef get_unique_rgn_ref(void);
OTF2_StringRef get_unique_str_ref(void);
OTF2_LocationRef get_unique_loc_ref(void);

#endif // OTTER_TRACE_UNIQUE_REFS_H
