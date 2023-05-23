/**
 * @file trace-state.h
 * @author Adam Tuft
 * @brief 
 * @version 0.1
 * @date 2023-04-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(OTTER_TRACE_STATE_TYPEDEF_H)
#define OTTER_TRACE_STATE_TYPEDEF_H

#include "public/types/string_value_registry.hpp"

/**
 * @brief Opaque handle to an Otter trace state.
 * 
 */
typedef struct trace_state_t trace_state_t;

string_registry *trace_state_get_string_registry(trace_state_t * state);
void trace_state_lock_string_registry(trace_state_t * state);
void trace_state_unlock_string_registry(trace_state_t * state);

#endif // OTTER_TRACE_STATE_TYPEDEF_H
