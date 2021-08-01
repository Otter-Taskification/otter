import igraph as ig
from itertools import chain, count
from collections import defaultdict
from otf2.events import Enter, Leave, ThreadTaskCreate
from typing import Callable, Any, List, AnyStr

"""
Contains assorted helper functions for __main__.py
"""


def chain_lists(lists):
    return list(chain(*lists))


def set_tuples(tuples):
    s = set(tuples)
    if len(s) == 1:
        return s.pop()
    else:
        return s


def pass_args(args):
    return args


def pass_single_executor(events, **kw):
    region_types = {e.attributes[kw['attr']['region_type']] for e in events}
    if region_types == {'single_other', 'single_executor'}:
        single_executor, = filter(lambda e: e.attributes[kw['attr']['region_type'] ]=='single_executor', events)
        return single_executor
    else:
        return events


def reject_task_create(events, **kw):
    events = [e for e in events if type(e) is not ThreadTaskCreate]
    if len(events) == 1:
        return events[0]
    else:
        return events


def attr_handler(events=pass_single_executor, ints=min, lists=chain_lists, tuples=set, **kw):
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


def label_clusters(vs: ig.VertexSeq, condition: Callable[[ig.Vertex],bool], key: [AnyStr, Callable[[ig.Vertex], Any]]) -> List[int]:
    """Return cluster labels where condition is true (and a unique vertex label otherwise), with cluster handle supplied via key"""
    if isinstance(key, str):
        s = key
        key = lambda v: v[s]
    vertex_counter = count()
    cluster_counter = count(start=sum(not condition(v) for v in vs))
    label = defaultdict(lambda: next(cluster_counter))
    return [label[key(v)] if condition(v) else next(vertex_counter) for v in vs]


def get_uid(event, **kw):
    if isinstance(event, list):
        s, = set([e.attributes[kw['attr']['unique_id']] for e in event])
        return s
    elif type(event) in [Enter, Leave, ThreadTaskCreate]:
        return event.attributes[kw['attr']['unique_id']]


def get_etid(event, **kw):
    if isinstance(event, list):
        s, = set([e.attributes[kw['attr']['encountering_task_id']] for e in event])
        return s
    elif type(event) in [Enter, Leave, ThreadTaskCreate]:
        return event.attributes[kw['attr']['encountering_task_id']]


def descendants_if(node, cond=lambda x: True):
    """Yield all descendants D of node, skipping E & its descendants if cond(E) is False."""
    for child in node.successors():
        if cond(child):
            yield from descendants_if(child, cond=cond)
    yield node.index
