import otf2

### Filter OTF2 region definitions

# def is_parallel_region(rgn):
#     return rgn.region_role in [otf2.RegionRole.PARALLEL]
#
# def is_task_region(rgn):
#     return rgn.region_role in [otf2.RegionRole.TASK]
#
# def is_initial_task_region(rgn):
#     return is_task_region(rgn) and "initial" in rgn.name
#
# def is_explicit_task_region(rgn):
#     return is_task_region(rgn) and "explicit" in rgn.name
#
# def is_global_region(rgn):
#     return is_parallel_region(rgn) or is_initial_task_region(rgn) or is_explicit_task_region(rgn)
#
# ### Filter Otter location maps based on attributes of the 1st event
#
# def is_parallel_context(lmap):
#     loc, evt, attr = next(iter(lmap))
#     return evt.attributes[attr['event_type']] == 'parallel_begin'
#
# def is_task_context(lmap):
#     loc, evt, attr = next(iter(lmap))
#     return evt.attributes[attr['event_type']] == 'task_enter'
#
# def is_global_context(lmap):
#     loc, evt, attr = next(iter(lmap))
#     return evt.attributes[attr['event_type']] in ['parallel_begin', 'task_enter']
#
# def is_initial_task_context(lmap):
#     loc, evt, attr = next(iter(lmap))
#     return evt.attributes[attr['event_type']] == 'task_enter' and "initial task" in evt.region.name
#
# def is_explicit_task_context(lmap):
#     loc, evt, attr = next(iter(lmap))
#     return evt.attributes[attr['event_type']] == 'task_enter' and "explicit task" in evt.region.name
#
# def is_root_initial_task_context(lmap):
#     loc, evt, attr = next(iter(lmap))
#     return evt.attributes[attr['event_type']] == 'task_enter' and evt.region.name == "initial task 0"
