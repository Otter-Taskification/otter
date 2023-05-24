/**
 * @file trace-initialise.h
 * @author Adam Tuft
 * @brief The public interface to initialise and finalise tracing
 * @version 0.1
 * @date 2023-03-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(OTTER_TRACE_INIT_H)
#define OTTER_TRACE_INIT_H

#include <stdbool.h>
#include "public/otter-common.h"
// #include "public/otter-trace/trace-state.h"

/**
 * @brief Initialise otter-trace according to the given options. Store the state
 * in `state`.
 */
bool trace_initialise(otter_opt_t *opt);

/**
 * @brief Finalise the provided otter-trace state
 */
bool trace_finalise(void);

#endif // OTTER_TRACE_INIT_H
