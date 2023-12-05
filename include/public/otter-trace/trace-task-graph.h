/**
 * @file trace-task-graph.h
 * @author Adam Tuft
 * @brief Ties together the includes for the components of otter-trace needed by
 * the otter-task-graph event source
 * @version 0.1
 * @date 2023-03-31
 *
 * @copyright Copyright (c) 2023
 *
 */

#if !defined(OTTER_TRACE_TASK_GRAPH_H)
#define OTTER_TRACE_TASK_GRAPH_H

#include "public/config.h"

#include "api/otter-task-graph/otter-task-graph.h" // only needed for otter_task_context typedef
#include "public/otter-common.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-region-attr.h"
#include "public/otter-trace/trace-types.h"

void trace_graph_event_task_create(trace_location_def_t *location,
                                   unique_id_t encountering_task_id,
                                   unique_id_t new_task_id,
                                   otter_string_ref_t task_label,
                                   otter_src_ref_t create_ref);

void trace_graph_event_task_begin(trace_location_def_t *location,
                                  unique_id_t encountering_task_id,
                                  otter_src_ref_t start_ref);

void trace_graph_event_task_end(trace_location_def_t *location,
                                unique_id_t encountering_task_id,
                                otter_src_ref_t end_ref);

void trace_graph_synchronise_tasks(trace_location_def_t *location,
                                   unique_id_t encountering_task_id,
                                   trace_sync_region_attr_t sync_attr,
                                   otter_endpoint_t endpoint);

void trace_graph_task_dependency(trace_location_def_t *location,
                                 unique_id_t pred, unique_id_t succ);

void trace_task_graph_finalise(void);

#endif // OTTER_TRACE_TASK_GRAPH_H
