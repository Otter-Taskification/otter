/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

OMPT-CORE.C

Contains the standard code necessary to connect a particular tool's set of
callbacks to the OMP runtime. Expects a tool to implement the functions in
ompt-generic.h and registers the tool's callbacks with the runtime, printing the
result to stderr.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>

#if defined(__INTEL_COMPILER)
#include <omp-tools.h>
#else
#include <ompt.h>
#endif

#include "ompt-core.h"         // Macro definitions
#include "ompt-tool-generic.h" // For the prototypes of tool_setup/tool_finalise

/* Entry & exit functions passed back to the OMP runtime */
static int ompt_initialise(ompt_function_lookup_t, int, ompt_data_t *);
static void ompt_finalise(ompt_data_t *);

/* The main entrypoint to the tool. This function is called by the OMP runtime
   and it returns pointers to the tool's entry & exit functions, as well as a
   pointer that is passed to both functions.
*/
ompt_start_tool_result_t *
ompt_start_tool(
    unsigned int omp_version,
    const char  *runtime_version)
{
    fprintf(stderr, "%s, OMP v. %u\n", runtime_version, omp_version);

    static ompt_start_tool_result_t result;
    result.initialize    = &ompt_initialise;
    result.finalize      = &ompt_finalise;
    result.tool_data.ptr = NULL;

    return &result;
}

/* Initialise the tool

   Called by the OMP runtime. The lookup argument allows us to lookup OMP 
   runtime functions. Request the callbacks the tool implements and register
   them with the runtime. Print the result of registering the callback. Not all
   callbacks may be available.
*/   
static int
ompt_initialise(
    ompt_function_lookup_t lookup, 
    int                    initial_device_num, 
    ompt_data_t *          tool_data)
{
    fprintf(stderr, "tool started\n");

    /* Passed into the tool for it to return function pointers to its
       callbacks, or NULL for those callbacks not required. */
    static tool_callbacks_t callbacks =
    {
        #define init_member_null(event, type, id) .on_##event = NULL,
        FOREACH_OMPT_EVENT(init_member_null)
    };

    tool_setup(&callbacks, lookup);
   
    ompt_set_callback_t ompt_set_callback = 
        (ompt_set_callback_t) lookup("ompt_set_callback");

    FOREACH_OMPT_EVENT(set_callback);

    return 1;
}

/* Tool exit - called just before OMP exits */
static void
ompt_finalise(
    ompt_data_t *tool_data)
{
    tool_finalise();
    fprintf(stderr, "tool finished\n");
    return;
}

