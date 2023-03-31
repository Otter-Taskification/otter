#if !defined(OTTER_TRACE_TASK_GRAPH_H)
#define OTTER_TRACE_TASK_GRAPH_H

#include "public/otter-common.h"
#include "api/otter-task-graph/otter-task-graph.h" // only needed for otter_task_context typedef
#include "public/otter-trace/trace-types.h"
#include "public/otter-trace/trace-location.h"
#include "public/otter-trace/trace-region-def.h"

// TODO: these declarations should be exposed in public/otter-trace/trace-archive.h
bool trace_initialise_archive(otter_opt_t *opt);
bool trace_finalise_archive(void);

// TODO: move these declarations to their own header under public/otter-trace
void trace_graph_event_task_begin(otter_task_context *task, trace_task_region_attr_t task_attr);
void trace_graph_event_task_end(otter_task_context *task);
void trace_graph_synchronise_tasks(otter_task_context *task, trace_sync_region_attr_t sync_attr);

#endif // OTTER_TRACE_TASK_GRAPH_H
