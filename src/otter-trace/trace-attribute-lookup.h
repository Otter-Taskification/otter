#if !defined(OTTER_TRACE_ATTRIBUTE_LOOKUP_H)
#define OTTER_TRACE_ATTRIBUTE_LOOKUP_H

#include "otter-trace/trace-attributes.h"
#include <otf2/OTF2_GeneralDefinitions.h>

/* Lookup tables mapping enum value to string ref */
extern OTF2_StringRef attr_name_ref[n_attr_defined][2];
extern OTF2_StringRef attr_label_ref[n_attr_label_defined];

#endif // OTTER_TRACE_ATTRIBUTE_LOOKUP_H
