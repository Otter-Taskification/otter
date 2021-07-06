#if !defined(OTTER_ENV_H)
#define OTTER_ENV_H

/* Define the environment variables OTTER looks for durng setup */

#define ENV_VAR_APPEND_HOST     "OTTER_APPEND_HOSTNAME"
#define ENV_VAR_TRACE_OUTPUT    "OTTER_OTF2_TRACE_NAME"
#define ENV_VAR_TRACE_PATH      "OTTER_OTF2_TRACE_PATH"
#define ENV_VAR_REPORT_CBK      "OTTER_REPORT_CALLBACKS"

/* Default values */
#define DEFAULT_OTF2_TRACE_OUTPUT "otter_trace"
#define DEFAULT_OTF2_TRACE_PATH   "trace"

#endif // OTTER_ENV_H