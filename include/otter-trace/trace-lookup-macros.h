#if !defined(OTTER_TRACE_LOOKUP_MACROS_H)
#define OTTER_TRACE_LOOKUP_MACROS_H

#define WORK_TYPE_TO_STR_REF(type)                                             \
   (type == ompt_work_loop                                                     \
        ? attr_label_ref[attr_region_type_loop]             :                  \
    type == ompt_work_sections                                                 \
        ? attr_label_ref[attr_region_type_sections]         :                  \
    type == ompt_work_single_executor                                          \
        ? attr_label_ref[attr_region_type_single_executor]  :                  \
    type == ompt_work_single_other                                             \
        ? attr_label_ref[attr_region_type_single_other]     :                  \
    type == ompt_work_workshare                                                \
        ? attr_label_ref[attr_region_type_workshare]        :                  \
    type == ompt_work_distribute                                               \
        ? attr_label_ref[attr_region_type_distribute]       :                  \
    type == ompt_work_taskloop                                                 \
        ? attr_label_ref[attr_region_type_taskloop]         : 0)

#define WORK_TYPE_TO_OTF2_REGION_ROLE(type)                                    \
   (type == ompt_work_loop             ? OTF2_REGION_ROLE_LOOP      :          \
    type == ompt_work_sections         ? OTF2_REGION_ROLE_SECTIONS  :          \
    type == ompt_work_single_executor  ? OTF2_REGION_ROLE_SINGLE    :          \
    type == ompt_work_single_other     ? OTF2_REGION_ROLE_SINGLE    :          \
    type == ompt_work_workshare        ? OTF2_REGION_ROLE_WORKSHARE :          \
    type == ompt_work_distribute       ? OTF2_REGION_ROLE_UNKNOWN   :          \
    type == ompt_work_taskloop         ? OTF2_REGION_ROLE_LOOP      :          \
        OTF2_REGION_ROLE_UNKNOWN)

/* Intel compiler defines additional types of sync regions compared to LLVM */
#if defined(__INTEL_COMPILER)
#define SYNC_TYPE_TO_STR_REF(type)                                             \
   (type == ompt_sync_region_barrier                                           \
        ? attr_label_ref[attr_region_type_barrier]                  :          \
    type == ompt_sync_region_barrier_implicit                                  \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_explicit                                  \
        ? attr_label_ref[attr_region_type_barrier_explicit]         :          \
    type == ompt_sync_region_barrier_implementation                            \
        ? attr_label_ref[attr_region_type_barrier_implementation]   :          \
    type == ompt_sync_region_taskwait                                          \
        ? attr_label_ref[attr_region_type_taskwait]                 :          \
    type == ompt_sync_region_taskgroup                                         \
        ? attr_label_ref[attr_region_type_taskgroup]                :          \
    type == ompt_sync_region_barrier_implicit_workshare                        \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_implicit_parallel                         \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_teams                                     \
        ? attr_label_ref[attr_region_type_barrier_implicit]         : 0 )
#else
#define SYNC_TYPE_TO_STR_REF(type)                                             \
   (type == ompt_sync_region_barrier                                           \
        ? attr_label_ref[attr_region_type_barrier]                  :          \
    type == ompt_sync_region_barrier_implicit                                  \
        ? attr_label_ref[attr_region_type_barrier_implicit]         :          \
    type == ompt_sync_region_barrier_explicit                                  \
        ? attr_label_ref[attr_region_type_barrier_explicit]         :          \
    type == ompt_sync_region_barrier_implementation                            \
        ? attr_label_ref[attr_region_type_barrier_implementation]   :          \
    type == ompt_sync_region_taskwait                                          \
        ? attr_label_ref[attr_region_type_taskwait]                 :          \
    type == ompt_sync_region_taskgroup                                         \
        ? attr_label_ref[attr_region_type_taskgroup] : 0)
#endif

#if defined(__INTEL_COMPILER)
#define SYNC_TYPE_TO_OTF2_REGION_ROLE(type)                                    \
   (type == ompt_sync_region_barrier                                           \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implicit                                  \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_explicit                                  \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implementation                            \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_taskwait                                          \
        ? OTF2_REGION_ROLE_TASK_WAIT :                                         \
    type == ompt_sync_region_taskgroup                                         \
        ? OTF2_REGION_ROLE_TASK_WAIT :                                         \
    type == ompt_sync_region_barrier_implicit_workshare                        \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_implicit_parallel                         \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_teams                                     \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :  OTF2_REGION_ROLE_UNKNOWN)
#else
#define SYNC_TYPE_TO_OTF2_REGION_ROLE(type)                                    \
   (type == ompt_sync_region_barrier                                           \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implicit                                  \
        ? OTF2_REGION_ROLE_IMPLICIT_BARRIER :                                  \
    type == ompt_sync_region_barrier_explicit                                  \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_barrier_implementation                            \
        ? OTF2_REGION_ROLE_BARRIER :                                           \
    type == ompt_sync_region_taskwait                                          \
        ? OTF2_REGION_ROLE_TASK_WAIT :                                         \
    type == ompt_sync_region_taskgroup                                         \
        ? OTF2_REGION_ROLE_TASK_WAIT : OTF2_REGION_ROLE_UNKNOWN)
#endif

#define TASK_TYPE_TO_STR_REF(type)                                             \
    (type == ompt_task_initial ? attr_label_ref[attr_task_type_initial] :      \
     type == ompt_task_implicit ? attr_label_ref[attr_task_type_implicit] :    \
     type == ompt_task_explicit ? attr_label_ref[attr_task_type_explicit] :    \
     type == ompt_task_target ? attr_label_ref[attr_task_type_target] : 0 )    \

#define TASK_STATUS_TO_STR_REF(status)                                         \
   (status == ompt_task_complete      ?                                        \
        attr_label_ref[attr_prior_task_status_complete] :                      \
    status == ompt_task_yield         ?                                        \
        attr_label_ref[attr_prior_task_status_yield] :                         \
    status == ompt_task_cancel        ?                                        \
        attr_label_ref[attr_prior_task_status_cancel] :                        \
    status == ompt_task_detach        ?                                        \
        attr_label_ref[attr_prior_task_status_detach] :                        \
    status == ompt_task_early_fulfill ?                                        \
        attr_label_ref[attr_prior_task_status_early_fulfil] :                  \
    status == ompt_task_late_fulfill  ?                                        \
        attr_label_ref[attr_prior_task_status_late_fulfil] :                   \
    status == ompt_task_switch        ?                                        \
        attr_label_ref[attr_prior_task_status_switch] : 0 )

#endif // OTTER_TRACE_LOOKUP_MACROS_H
