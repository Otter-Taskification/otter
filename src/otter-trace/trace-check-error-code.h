#if !defined(OTTER_TRACE_OTF2_ERROR_CODE_H)
#define OTTER_TRACE_OTF2_ERROR_CODE_H

#include <otf2/OTF2_ErrorCodes.h>
#include "public/debug.h"

#define CHECK_OTF2_ERROR_CODE(r)                                               \
    {if (r != OTF2_SUCCESS)                                                    \
    {                                                                          \
        LOG_ERROR("%s: %s (%s:%d)",                                            \
            OTF2_Error_GetName(r),                                             \
            OTF2_Error_GetDescription(r),                                      \
            __FILE__,                                                          \
            __LINE__                                                           \
        );                                                                     \
    }}

#endif // OTTER_TRACE_OTF2_ERROR_CODE_H
