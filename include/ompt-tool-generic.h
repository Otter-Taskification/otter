#if !defined(OMPT_TOOL_GENERIC_H)
#define OMPT_TOOL_GENERIC_H

#include <ompt.h>

/* Defines how any OMPT tool can register callbacks via ompt-core. The tool
   should implement tool_setup so that it registers its callbacks with 
   ompt-core by setting the relevant function pointers in the "implemented"
   struct. tool_finalise is called by ompt-core once all OMP regions are 
   complete and the program is about to exit.
*/

/* Define the struct used by ompt-core which is passed to a tool for it to
   return pointers to its implemented OMP callbacks. Contains one field for each
   OMPT event defined in ompt.h e.g. the on_ompt_callback_thread_begin field
   indicates the callback for the ompt_callback_thread_begin event.
*/
typedef struct tool_callbacks_t {
#define make_member(event, fn_signature, id) fn_signature on_##event;
  FOREACH_OMPT_EVENT(make_member)
} tool_callbacks_t;

/* Register a particular tool's callbacks with ompt-core

   A tool should implement tool_setup to populate 'implemented' with pointers to
   the callbacks it implements. The tool can use the 'lookup' function pointer
   to get OMPT function pointers.
 */
void tool_setup(tool_callbacks_t *implemented, ompt_function_lookup_t lookup);

/* Finalise the tool once OMP finished */
void tool_finalise(void);

#endif // OMPT_TOOL_GENERIC_H
