import argparse
import warnings
import igraph as ig
import otf2
from itertools import chain, zip_longest, cycle, count
from collections import defaultdict, deque
from otf2.events import Enter, Leave, ThreadTaskCreate
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


def process_chunk(chunk, color_map=None):
    """Return a tuple of chunk type, task-create links and the chunk's graph"""
    print(chunk)
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
                and node['event'].attributes[chunk.attr['region_type']] in ['single_executor', 'single_other'])\
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
            g.add_edge(node, dummy_node)
            task_create_nodes.append(dummy_node)
            continue
        elif len(task_create_nodes) > 0:
            num_tc_nodes = len(task_create_nodes)
            for _ in range(num_tc_nodes):
                g.add_edge(task_create_nodes.pop(), node)

        prior_node = node

    return chunk_type, task_links, g


def chain_lists(lists):
    return list(chain(*lists))


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
    event_types = {type(e) for e in events}
    events = [e for e in events if type(e) is not ThreadTaskCreate]
    print(events)
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
    print("reading chunks")
    with otf2.reader.open(anchorfile) as tr:
        attr = trace.AttributeLookup(tr.definitions.attributes)
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
    chunk_counter = defaultdict(int)
    for c in chunk_types:
        chunk_counter[c] += 1

    print("graph chunks created:")
    for k, v in chunk_counter.items():
        print(f"  {k:18s} {v:8d}")

    # Collect all chunks
    g = ig.disjoint_union(g_list)

    # Collapse by parallel sequence ID
    vertex_counter = count()
    condition = lambda v: v['parallel_sequence_id'] is not None
    parallel_cluster_counter = count(start=sum([not(condition(v)) for v in g.vs]))
    relabel = defaultdict(lambda: next(parallel_cluster_counter))
    g.vs['cluster'] = [relabel[v['parallel_sequence_id']] if condition(v) else next(vertex_counter) for v in g.vs]
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(attr=attr))

    # Collapse by single-begin/end event
    vertex_counter = count()
    condition = lambda v: v['event'].attributes[attr['region_type']]=='single_executor' if type(v['event']) in [Enter, Leave] else False
    single_cluster_counter = count(start=sum([not(condition(v)) for v in g.vs]))
    relabel = defaultdict(lambda: next(single_cluster_counter))
    g.vs['cluster'] = [relabel[v['event']] if condition(v) else next(vertex_counter) for v in g.vs]
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(attr=attr))

    # Search for all explicit-task-enter nodes
    is_task_enter_leave = lambda v: v['event'].attributes[attr['region_type']]=='explicit_task' if type(v['event']) in [Enter, Leave] else False
    task_nodes = list(g.vs.select(is_task_enter_leave))

    num_chunks = defaultdict(int)
    for n in task_nodes:
        if n['event'].attributes[attr['endpoint']] == 'enter':
            num_chunks[n['event'].attributes[attr['unique_id']]] += 1

    task_chunk_sequences = defaultdict(deque)
    for n in task_nodes:
        task_id = n['event'].attributes[attr['unique_id']]
        if num_chunks[task_id] > 1:
            task_chunk_sequences[task_id].append(n)

    for k in task_chunk_sequences:
        task_chunk_sequences[k] = deque(sorted(task_chunk_sequences[k], key=lambda v: v['event'].time))

    # Collapse by task-enter/leave event
    # vertex_counter = count()
    # condition = lambda v: v['task_cluster_id'] is not None
    # for v in g.vs:
    #     if type(v['event']) in [Enter, Leave] and v['event'].attributes[attr['region_type']]=='explicit_task':
    #         v['task_cluster_id'] = (v['event'].attributes[attr['unique_id']], v['event'].attributes[attr['endpoint']])
    # task_cluster_counter = count(start=sum([not(condition(v)) for v in g.vs]))
    # relabel = defaultdict(lambda: next(task_cluster_counter))
    # g.vs['cluster'] = [relabel[v['task_cluster_id']] if v['task_cluster_id'] is not None else next(vertex_counter) for v in g.vs]
    # g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(events=reject_task_create, attr=attr))

    ### TO DO ###
    """
    - handle multiple sequential chunks for explicit tasks
    - collapse duplicate nodes for barriers, taskwaits etc
    - apply task synchronisation edges
    """

    # Unpack the region_type attribute
    for v in g.vs:
        if isinstance(v['event'], list):
            v['region_type'], = set([e.attributes[attr['region_type']] for e in v['event']])
            v['endpoint'], = set([e.attributes[attr['endpoint']] for e in v['event']])
        else:
            v['region_type'] = v['event'].attributes[attr['region_type']]
            v['endpoint'] = v['event'].attributes[attr['endpoint']]

    # g.vs['color'] = [cmap[v['region_type']] for v in g.vs]
    g.vs['color'] = [cmap[v['endpoint']] for v in g.vs]
    g.vs['style'] = 'filled'
    g.vs['shape'] = 'circle'

    for n in task_nodes:
        if num_chunks[n['event'].attributes[attr['unique_id']]] > 1:
            # n['color'] = 'red'
            n['label'] = n['event'].attributes[attr['unique_id']]

    for k, v in task_chunk_sequences.items():
        print(k)
        for n in v:
            print(n['event'].time, n['event'].attributes[attr['endpoint']], n['event'].attributes[attr['unique_id']], n['event'].attributes[attr['encountering_task_id']])

    g.simplify()

    with otf2.reader.open(anchorfile) as tr:
        attr = trace.AttributeLookup(tr.definitions.attributes)
        lmap = trace.LocationEventMap(tr.events, attr)

    with otf2.reader.open(anchorfile) as tr:
        for chunk in yield_chunks(tr):
            print(chunk)
