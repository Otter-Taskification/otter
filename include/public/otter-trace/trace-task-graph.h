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

#include "public/otter-common.h"
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-region-attr.h"
#include "api/otter-task-graph/otter-task-graph.h" // only needed for otter_task_context typedef

void trace_graph_event_task_begin(otter_task_context *task, trace_task_region_attr_t task_attr);
void trace_graph_event_task_end(otter_task_context *task);
void trace_graph_synchronise_tasks(otter_task_context *task, trace_sync_region_attr_t sync_attr);

#endif // OTTER_TRACE_TASK_GRAPH_H
