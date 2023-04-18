/**
 * @file trace-state.h
 * @author Adam Tuft
 * @brief Defines the interface for interacting with the trace_state struct
 * @version 0.1
 * @date 2023-04-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <otf2/OTF2_Archive.h>
#include <otf2/OTF2_GlobalDefWriter.h>
#include <public/types/string_value_registry.hpp>
#include <public/otter-trace/trace-state.h>

trace_state_t *trace_state_new(OTF2_Archive *archive, OTF2_GlobalDefWriter *global_def_writer, string_registry *registry);
void trace_state_destroy(trace_state_t *state);

OTF2_Archive *trace_state_get_archive(trace_state_t * state);

OTF2_GlobalDefWriter *trace_state_get_global_def_writer(trace_state_t * state);
void trace_state_lock_global_def_writer(trace_state_t * state);
void trace_state_unlock_global_def_writer(trace_state_t * state);

string_registry *trace_state_get_string_registry(trace_state_t * state);
void trace_state_lock_string_registry(trace_state_t * state);
void trace_state_unlock_string_registry(trace_state_t * state);
