#if !defined(OTTER_ENTRY_H)
#define OTTER_ENTRY_H

#include "private/otter-ompt/otter-ompt-header.h"
#include "public/otter-common.h"

/* Define the struct used by otter-entry which is passed to otter for it to
   return pointers to its implemented OMP callbacks. Contains one field for each
   OMPT event defined in ompt.h e.g. the on_ompt_callback_thread_begin field
   indicates the callback for the ompt_callback_thread_begin event.
*/
typedef struct tool_callbacks_t {
#define make_member(event, fn_signature, id) fn_signature on_##event;
  FOREACH_OMPT_EVENT(make_member)
} tool_callbacks_t;

/* Pass callbacks back to otter-entry to be registered with OMP runtime
Return static pointer to otter_opt_t struct */
otter_opt_t *tool_setup(tool_callbacks_t *implemented, ompt_function_lookup_t lookup);

/* Finalise the tool once OMP finished */
void tool_finalise(ompt_data_t *tool_data);

#endif // OTTER_ENTRY_H
