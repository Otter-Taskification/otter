#if !defined(OTTER_TRACE_LOOKUP_MACROS_H)
#define OTTER_TRACE_LOOKUP_MACROS_H

#include "otter-trace/trace-region-types.h"
#include "otter-trace/trace-attributes.h"

static inline attr_label_enum_t work_type_as_label(otter_work_t work_type)
{
    switch (work_type)
    {
        case otter_work_loop:             return attr_region_type_loop;
        case otter_work_sections:         return attr_region_type_sections;
        case otter_work_single_executor:  return attr_region_type_single_executor;
        case otter_work_single_other:     return attr_region_type_single_other;
        case otter_work_workshare:        return attr_region_type_workshare;
        case otter_work_distribute:       return attr_region_type_distribute;
        case otter_work_taskloop:         return attr_region_type_taskloop;
        default:
            return attr_label_string_not_defined;
    }
}

static inline attr_label_enum_t sync_type_as_label(otter_sync_region_t sync_type)
{
    switch (sync_type)
    {
    case otter_sync_region_barrier:                return attr_region_type_barrier;
    case otter_sync_region_barrier_implicit:       return attr_region_type_barrier_implicit;
    case otter_sync_region_barrier_explicit:       return attr_region_type_barrier_explicit;
    case otter_sync_region_barrier_implementation: return attr_region_type_barrier_implementation;
    case otter_sync_region_taskwait:               return attr_region_type_taskwait;
    case otter_sync_region_taskgroup:              return attr_region_type_taskgroup;
    default:
        return attr_label_string_not_defined;
    }
}

static inline attr_label_enum_t task_type_as_label(otter_task_flag_t task_type)
{
    switch (task_type & otter_task_type_mask) {
        case otter_task_initial:  return attr_task_type_initial_task;
        case otter_task_implicit: return attr_task_type_implicit_task;
        case otter_task_explicit: return attr_task_type_explicit_task;
        case otter_task_target:   return attr_task_type_target_task;
        default:
            return attr_label_string_not_defined;
    }
}

static inline attr_label_enum_t task_status_as_label(otter_task_status_t task_status)
{
    switch (task_status)
    {
        case otter_task_state_undef:   return attr_prior_task_status_undefined;
        case otter_task_complete:      return attr_prior_task_status_complete;
        case otter_task_yield:         return attr_prior_task_status_yield;
        case otter_task_cancel:        return attr_prior_task_status_cancel;
        case otter_task_detach:        return attr_prior_task_status_detach;
        case otter_task_early_fulfill: return attr_prior_task_status_early_fulfil;
        case otter_task_late_fulfill:  return attr_prior_task_status_late_fulfil;
        case otter_task_switch:        return attr_prior_task_status_switch;
        default:
            return attr_label_string_not_defined;
    }
}

static inline attr_label_enum_t thread_type_as_label(otter_thread_t thread_type)
{
    switch (thread_type)
    {
        case otter_thread_initial:  return attr_thread_type_initial;
        case otter_thread_worker:   return attr_thread_type_worker;
        default:
            return attr_label_string_not_defined;
    }
}

static inline attr_label_enum_t region_type_as_label(trace_region_type_t region_type, trace_region_attr_t attr)
{
    switch (region_type)
    {
        case trace_region_parallel:    return attr_region_type_parallel;
        case trace_region_workshare:   return work_type_as_label(attr.wshare.type);
        case trace_region_synchronise: return sync_type_as_label(attr.sync.type);
        case trace_region_task:        return task_type_as_label(attr.task.type);
        case trace_region_master:      return attr_region_type_master;
        case trace_region_phase:       return attr_region_type_generic_phase;
        default:
            return attr_label_string_not_defined;
    }
}

#endif // OTTER_TRACE_LOOKUP_MACROS_H
