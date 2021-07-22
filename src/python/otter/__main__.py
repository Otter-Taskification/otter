import argparse
import warnings
import igraph as ig
import otf2
from itertools import chain, zip_longest, cycle, count
from collections import defaultdict, deque, Counter
from otf2.events import Enter, Leave, ThreadTaskCreate
from typing import Callable, Iterable, Any, List, Set, Tuple, AnyStr, Union
from . import trace
from . import filters as ft


def plot_graph(g, layout=None, **kwargs):
    if layout is None:
        layout = g.layout_sugiyama()
    ig.plot(g, layout=layout, **kwargs)


def graph_to_dot(g, **kwargs):

    g.vs['region_type'] = [r if r != "" else "undefined" for r in g.vs['region_type']]
    g.vs['sync_type'] = [r if r != "" else "undefined" for r in g.vs['sync_type']]

    for v in g.vs:
        # Fix some types for dot file format
        v['parent_task_id'] = str(v['parent_task_id'])
        if v['parent_task_type'] in [None, ""]:
            v['parent_task_type'] = 'undefined'
        if v['prior_task_status'] in [None, ""]:
            v['prior_task_status'] = 'undefined'
        if v['event_type'] == 'task_enter':
            v['color'] = "limegreen"
        elif v['event_type'] == 'task_leave':
            v['color'] = "lightblue"

        # Style attributes
        v['style'] = 'filled'
        v['shape'] = {'parallel': 'hexagon',
                      'taskwait': 'octagon',
                      'taskgroup': 'hexagon',
                      'taskloop': 'circle',
                      'single_executor': 'diamond'}.get(v['region_type'], "rectangle")

        # Set label for task-enter and task-leave nodes
        v['label'] = " " if v['unique_id'] is None else str(v['unique_id'])

        # Set label for all other nodes
        v['label'] = {'parallel': "{}".format(v['unique_id']),
                      'taskwait': 'tw',
                      'taskgroup': 'tg',
                      'taskloop': 'tl',
                      'loop': 'lp',
                      'barrier_implicit': 'ib',
                      'single_executor': 'sn'}.get(v['region_type'], v['label'])

        # Default edge colour if none set
        for e in g.es:
            if e['color'] in [None, ""]:
                e['color'] = 'black'

    fname = kwargs.get('target', 'graph.dot')
    print("Writing graph to '{}'".format(fname))

    # Ignore runtime warnings emitted due to types being ignored/changed by igraph
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        g.write_dot(fname)


def graph_to_graphml(g, **kwargs):
    fname = kwargs.get('target', 'graph.graphml')
    print("Writing graph to '{}'".format(fname))
    g.write_graphml(fname)


def yield_chunks(tr):
    attr = trace.AttributeLookup(tr.definitions.attributes)
    lmap_dict = defaultdict(lambda : trace.LocationEventMap(list(), attr))
    stack_dict = defaultdict(deque)
    for location, event in tr.events:
        if type(event) in [otf2.events.ThreadBegin, otf2.events.ThreadEnd]:
            continue
        if trace.event_defines_new_chunk(event, attr):
            # Event marks transition from one chunk to another
            if isinstance(event, Enter):
                if event.attributes.get(attr['region_type'], "") != 'explicit_task':
                    lmap_dict[location].append(location, event)
                stack_dict[location].append(lmap_dict[location])
                # New location map for new chunk
                lmap_dict[location] = trace.LocationEventMap([(location, event)], attr)
            elif isinstance(event, Leave):
                lmap_dict[location].append(location, event)
                yield lmap_dict[location]
                # Continue with enclosing chunk
                lmap_dict[location] = stack_dict[location].pop()
                if event.attributes.get(attr['region_type'], "") != 'explicit_task':
                    lmap_dict[location].append(location, event)
            else:
                lmap_dict[location].append(location, event)
        else:
            # Append event to current chunk for this location
            lmap_dict[location].append(location, event)


def process_chunk(chunk, color_map=None, print_chunk=None):
    """Return a tuple of chunk type, task-create links and the chunk's graph"""
    if color_map is None:
        color_map = defaultdict(lambda: 'gray')
    location, = chunk.locations()
    first_event, *events, last_event = chunk[location]
    chunk_type = first_event.attributes[chunk.attr['region_type']]
    g = ig.Graph(directed=True)
    prior_node = g.add_vertex(event=first_event)
    prior_node['color'] = color_map[first_event.attributes[chunk.attr['region_type']]]
    if chunk_type == 'parallel':
        parallel_id = first_event.attributes[chunk.attr['unique_id']]
        prior_node["parallel_sequence_id"] = (parallel_id, first_event.attributes[chunk.attr['endpoint']])
    task_create_nodes = deque()
    task_links = deque()
    k = 1
    for event in chain(events, (last_event,)):

        if event.attributes[chunk.attr['region_type']] in ['implicit_task']:
            continue

        node = g.add_vertex(event=event)

        # Label nodes in a parallel chunk by their position for easier merging
        if chunk_type == 'parallel' and event.attributes[chunk.attr['event_type']] != "task_create":
            node["parallel_sequence_id"] = (parallel_id, k)
            k += 1

        if node['event'].attributes[chunk.attr['region_type']]=='parallel':
            # Label nested parallel regions for easier merging...
            if event is not last_event:
                node["parallel_sequence_id"] = (node['event'].attributes[chunk.attr['unique_id']], node['event'].attributes[chunk.attr['endpoint']])
            # ... but distinguish from a parallel chunks's final parallel-end event
            else:
                node["parallel_sequence_id"] = (parallel_id, node['event'].attributes[chunk.attr['endpoint']])

        # Add edge except for (single begin -> single end) and (parallel begin -> parallel end)
        if not(prior_node['event'].attributes[chunk.attr['region_type']] in ['single_executor', 'single_other'] \
                and prior_node['event'].attributes[chunk.attr['endpoint']]=='enter' \
                and node['event'].attributes[chunk.attr['region_type']] in ['single_executor', 'single_other']\
                and node['event'].attributes[chunk.attr['endpoint']]=='leave')\
            and not(prior_node['event'].attributes[chunk.attr['endpoint']]=='enter' \
                and prior_node['event'].attributes[chunk.attr['region_type']]=='parallel'\
                and node['event'].attributes[chunk.attr['endpoint']]=='leave' \
                and node['event'].attributes[chunk.attr['region_type']]=='parallel'\
                and node['event'].attributes[chunk.attr['unique_id']]==prior_node['event'].attributes[chunk.attr['unique_id']]):
            g.add_edge(prior_node, node)

        # Add task links
        if event.attributes[chunk.attr['event_type']] in ["task_create", "task_enter"]:
            task_links.append((event.attributes[chunk.attr['encountering_task_id']], event.attributes[chunk.attr['unique_id']]))

        # For task_create add dummy nodes for easier merging
        if event.attributes[chunk.attr['event_type']] == "task_create":
            node['task_cluster_id'] = (event.attributes[chunk.attr['unique_id']], 'enter')
            dummy_node = g.add_vertex(event=event, task_cluster_id=(event.attributes[chunk.attr['unique_id']], 'leave'))
            task_create_nodes.append(dummy_node)
            continue
        elif len(task_create_nodes) > 0:
            num_tc_nodes = len(task_create_nodes)
            for _ in range(num_tc_nodes):
                g.add_edge(task_create_nodes.pop(), node)

        prior_node = node

    if chunk_type == 'explicit_task' and len(events) == 0:
        g.delete_edges([0])

    # Require at least 1 edge between start and end nodes if there are no internal nodes, except for empty explicit task chunks
    if chunk_type != "explicit_task" and len(events) == 0 and g.ecount() == 0:
        g.add_edge(g.vs[0], g.vs[1])

    if print_chunk is not None and len(events) > 0:
        print_chunk(chunk)

    return chunk_type, task_links, g


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
        single_executor, = filter(lambda e: e.attributes[attr['region_type'] ]=='single_executor', events)
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


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description='Convert an Otter OTF2 trace archive to its execution graph representation',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('anchorfile', help='OTF2 anchor file')
    args = parser.parse_args()
    anchorfile = args.anchorfile

    # Map region type to node color
    cmap = defaultdict(lambda: 'white', **{
        'initial_task':      'green',
        'implicit_task':     'fuchsia',
        'explicit_task':     'cyan',
        'parallel':          'yellow',
        'single_executor':   'blue',
        'single_other':      'orange',
        'taskwait':          'pink',
        'taskgroup':         'purple',
        'barrier_implicit':  'darkgreen',

        # Workshare regions
        'loop':              'brown',
        'taskloop':          'darkred',

        # For colouring by endpoint
        'enter':             'green',
        'leave':             'red'
    })

    cmap_event_type = defaultdict(lambda: 'white', **{
        str(Enter):               'green',
        str(Leave):               'red',
        str(ThreadTaskCreate):    'orange',
    })

    # Convert event stream into graph chunks
    print(f"loading OTF2 anchor file: {anchorfile}")
    print("generating chunks from event stream...")
    with otf2.reader.open(anchorfile) as tr:
        attr = trace.AttributeLookup(tr.definitions.attributes)
        regions = trace.RegionLookup(tr.definitions.regions)
        results = (process_chunk(chunk) for chunk in yield_chunks(tr))
        items = zip(*(results))
        chunk_types = next(items)
        task_links = next(items)
        g_list = next(items)

    # Task tree showing parent-child links
    task_tree = ig.Graph(edges=chain(*task_links), directed=True)
    task_tree.vs['color'] = 'green'
    task_tree.vs['shape'] = 'square'
    task_tree.vs['label'] = [v.index for v in task_tree.vs]
    tt_layout = task_tree.layout_reingold_tilford()

    # Count chunks by type
    print("graph chunks created:")
    for k, v in Counter(chunk_types).items():
        print(f"  {k:18s} {v:8d}")

    # Collect all chunks
    g = ig.disjoint_union(g_list)
    num_nodes = len(g.vs)

    print("{:20s} {:6d}".format("nodes created", num_nodes))

    if 'task_cluster_id' not in g.vs.attribute_names():
        g.vs['task_cluster_id'] = None

    # Collapse by parallel sequence ID
    g.vs['cluster'] = label_clusters(g.vs, lambda v: v['parallel_sequence_id'] is not None, 'parallel_sequence_id')
    nodes_before = num_nodes
    print("contracting by parallel sequence ID")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(attr=attr))
    num_nodes = len(g.vs)
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    # Collapse by single-begin/end event
    is_single_executor = lambda v: v['event'].attributes[attr['region_type']]=='single_executor' if type(v['event']) in [Enter, Leave] else False
    g.vs['cluster'] = label_clusters(g.vs, is_single_executor, 'event')
    nodes_before = num_nodes
    print("contracting by single-begin/end event")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(attr=attr))
    num_nodes = len(g.vs)
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    # Collapse by (task-ID, endpoint) to get 1 subgraph per task
    for v in g.vs:
        if type(v['event']) in [Enter, Leave] and v['event'].attributes[attr['region_type']]=='explicit_task':
            v['task_cluster_id'] = (v['event'].attributes[attr['unique_id']], v['event'].attributes[attr['endpoint']])
    g.vs['cluster'] = label_clusters(g.vs, lambda v: v['task_cluster_id'] is not None, 'task_cluster_id')
    nodes_before = num_nodes
    print("contracting by task ID & endpoint")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(events=reject_task_create, tuples=set_tuples, attr=attr))
    num_nodes = len(g.vs)
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    # Collapse by task ID where there are no links between to combine task nodes with nothing nested within
    def is_empty_task_region(v):
        if v['task_cluster_id'] is None:
            return False
        if type(v['event']) in [Enter, Leave]:
            return (isinstance(v['event'], Leave) and v.indegree()==0) or \
                   (isinstance(v['event'], Enter) and v.outdegree()==0)
        if isinstance(v['event'], list) and set(map(type, v['event'])) in [{Enter}, {Leave}]:
            return (set(map(type, v['event'])) == {Leave} and v.indegree()==0) or \
                   (set(map(type, v['event'])) == {Enter} and v.outdegree()==0)
    g.vs['cluster'] = label_clusters(g.vs, is_empty_task_region, lambda v: v['task_cluster_id'][0])
    nodes_before = num_nodes
    print("contracting by task ID where there are no nested nodes")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(events=reject_task_create, tuples=set_tuples, attr=attr))
    num_nodes = len(g.vs)
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    # Collapse redundant sync-enter/leave node pairs by labelling unique pairs of nodes identified by their shared edge
    dummy_counter = count()
    for e in g.es:
        node_types = set()
        for v in (e.source_vertex, e.target_vertex):
            if type(v['event']) is not list:
                node_types.add(v['event'].attributes[attr['region_type']])
            else:
                for item in v['event']:
                    node_types.add(item.attributes[attr['region_type']])
        if node_types in [{'barrier_implicit'}, {'barrier_explicit'}, {'taskwait'}] and \
                e.source_vertex.attributes().get('sync_cluster_id', None) is None and \
                e.target_vertex.attributes().get('sync_cluster_id', None) is None:
            value = next(dummy_counter)
            e.source_vertex['sync_cluster_id'], e.target_vertex['sync_cluster_id'] = value, value
    g.vs['cluster'] = label_clusters(g.vs, lambda v: v['sync_cluster_id'] is not None, lambda v: v['sync_cluster_id'])
    nodes_before = num_nodes
    print("contracting redundant sync-enter/leave node pairs")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(tuples=set_tuples, attr=attr))
    num_nodes = len(g.vs)
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    ### TO DO ###
    """
    - handle multiple sequential chunks for explicit tasks
    - apply task synchronisation edges
    """

    # Unpack the region_type attribute
    for v in g.vs:
        if isinstance(v['event'], list):
            v['region_type'], = set([e.attributes[attr['region_type']] for e in v['event']])
            v['endpoint'] = set([e.attributes[attr['endpoint']] for e in v['event']])
        else:
            v['region_type'] = v['event'].attributes[attr['region_type']]
            v['endpoint'] = v['event'].attributes[attr['endpoint']]

    g.vs['color'] = [cmap[v['region_type']] for v in g.vs]
    g.vs['style'] = 'filled'
    g.vs['shape'] = 'circle'
    g.vs['label'] = " "

    g.simplify()
