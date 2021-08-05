from itertools import chain, count
from collections import defaultdict
from otf2.events import Enter, Leave, ThreadTaskCreate

"""
Contains assorted helper functions for __main__.py and trace.py
"""


def attr_getter(attr_lookup):
    """Make a function to lookup an attribute of some event by its name"""
    def event_attr_getter(evt, name):
        if type(evt) is list:
            result, = set([e.attributes[attr_lookup[name]] for e in evt])
            return result
        elif type(evt) in [Enter, Leave, ThreadTaskCreate]:
            return evt.attributes[attr_lookup[name]]
        else:
            raise TypeError(f"unexpected type: {type(evt)}")
    return event_attr_getter


# Helpers related to manipulating the nodes of an igraph.Graph:


def chain_lists(lists):
    """Possible argument to attr_handler()"""
    return list(chain(*lists))


def set_tuples(tuples):
    """Possible argument to attr_handler()"""
    s = set(tuples)
    if len(s) == 1:
        return s.pop()
    else:
        return s


def pass_args(args):
    """Possible argument to attr_handler()"""
    return args


def pass_single_executor(events, **kw):
    """Possible argument to attr_handler()"""
    region_types = {e.attributes[kw['attr']['region_type']] for e in events}
    if region_types == {'single_other', 'single_executor'}:
        single_executor, = filter(lambda e: e.attributes[kw['attr']['region_type'] ]=='single_executor', events)
        return single_executor
    else:
        return events

def pass_master_event(events, **kw):
    """Possible argument to attr_handler()"""
    region_types = {e.attributes[kw['attr']['region_type']] for e in events}
    if region_types == {'master'} and len(set(events)) == 1:
        return events[0]
    else:
        return events


def reject_task_create(events, **kw):
    """Possible argument to attr_handler()"""
    events = [e for e in events if type(e) is not ThreadTaskCreate]
    if len(events) == 1:
        return events[0]
    else:
        return events


def attr_handler(events=pass_single_executor, ints=min, lists=chain_lists, tuples=set, **kw):
    """Make a function for combining lists of node attributes according to their type"""
    def attr_combiner(args):
        if len(args) == 1:
            return args[0]
        else:
            if all([isinstance(obj, int) for obj in args]):
                return ints(args)
            elif all([isinstance(obj, list) for obj in args]):
                return lists(args)
            elif all([isinstance(obj, tuple) for obj in args]):
                return tuples(args)
            elif all([type(obj) in [Enter, Leave, ThreadTaskCreate] for obj in args]):
                return events(args, **kw)
            else:
                return args[0]
    return attr_combiner


def label_clusters(vs, condition, key):
    """Return cluster labels (given by key function) where condition is true, or a unique vertex label otherwise"""
    if isinstance(key, str):
        s = key
        key = lambda v: v[s]
    vertex_counter = count()
    cluster_counter = count(start=sum(not condition(v) for v in vs))
    label = defaultdict(lambda: next(cluster_counter))
    return [label[key(v)] if condition(v) else next(vertex_counter) for v in vs]


# Functions useful in one specific place in the code:

def descendants_if(node, cond=lambda x: True):
    """Yield all descendants D of node, skipping E & its descendants if cond(E) is False."""
    for child in node.successors():
        if cond(child):
            yield from descendants_if(child, cond=cond)
    yield node.index


def events_bridge_region(previous, current, types, getter):
    """Used in trace.process_chunk to check for certain enter-leave event sequences"""
    return (getter(previous, 'region_type') in types
            and getter(previous, 'endpoint') == 'enter'
            and getter(current, 'region_type') in types
            and getter(current, 'endpoint') == 'leave')
