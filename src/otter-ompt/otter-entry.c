#include "otter-entry.h"
#include "callback.h"
#include "public/debug.h"
#include "public/otter-environment-variables.h"
#include "public/otter-version.h"
#include <omp-tools.h>
#include <stdio.h>
#include <stdlib.h>

/* Entry & exit functions passed back to the OMP runtime */
static int ompt_initialise(ompt_function_lookup_t, int, ompt_data_t *);
static void ompt_finalise(ompt_data_t *);

/* The main entrypoint to the tool. This function is called by the OMP runtime
   and it returns pointers to the tool's entry & exit functions, as well as a
   pointer that is passed to both functions.
*/
ompt_start_tool_result_t *ompt_start_tool(unsigned int omp_version,
                                          const char *runtime_version) {
  fprintf(stderr, "%s, OMP v. %u\n", runtime_version, omp_version);
  fprintf(stderr, "Otter v%s.%s.%s was compiled with %s\n", OTTER_VERSION_MAJOR,
          OTTER_VERSION_MINOR, OTTER_VERSION_PATCH, CC_VERSION);

  static ompt_start_tool_result_t result;
  result.initialize = &ompt_initialise;
  result.finalize = &ompt_finalise;
  result.tool_data.ptr = NULL;

  return &result;
}

/* Initialise the tool

   Called by the OMP runtime. The lookup argument allows us to lookup OMP
   runtime functions. Request the callbacks the tool implements and register
   them with the runtime. Print the result of registering the callback. Not all
   callbacks may be available.
*/
static int ompt_initialise(ompt_function_lookup_t lookup,
                           int initial_device_num, ompt_data_t *tool_data) {
  fprintf(stderr, "Starting OTTer...\n");

  /* Passed into the tool for it to return function pointers to its
     callbacks, or NULL for those callbacks not required. */
  static tool_callbacks_t callbacks = {
#define init_member_null(event, type, id) .on_##event = NULL,
      FOREACH_OMPT_EVENT(init_member_null)};

  tool_data->ptr = (ompt_data_t *)tool_setup(&callbacks, lookup);

  ompt_set_callback_t ompt_set_callback =
      (ompt_set_callback_t)lookup("ompt_set_callback");

  if (getenv(ENV_VAR_REPORT_CBK) != NULL) {
    fprintf(stderr, "\nReturn codes from ompt_set_callback:\n");
    fprintf(stderr, "%-32s | %s\n", "Event", "Availability");
    FOREACH_OMPT_EVENT(report_callback);
  }

  fprintf(stderr, "\nRegistering callbacks:\n");
  FOREACH_OMPT_EVENT(set_callback);

  return 1;
}

/* Tool exit - called just before OMP exits */
static void ompt_finalise(ompt_data_t *tool_data) {
  tool_finalise(tool_data);
  return;
}
