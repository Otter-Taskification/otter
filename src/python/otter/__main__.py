import argparse
import warnings
import igraph as ig
import otf2
from itertools import chain, count, groupby
from collections import defaultdict, deque, Counter
from otf2.events import Enter, Leave, ThreadTaskCreate
from typing import Callable, Any, List, AnyStr
from . import trace


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


def process_chunk(chunk, verbose=False):
    """Return a tuple of chunk type, task-create links and the chunk's graph"""
    location, = chunk.locations()
    first_event, *events, last_event = chunk[location]
    chunk_type = first_event.attributes[chunk.attr['region_type']]
    g = ig.Graph(directed=True)
    prior_node = g.add_vertex(event=first_event)
    taskgroup_enter_event = None
    if chunk_type == 'parallel':
        parallel_id = first_event.attributes[chunk.attr['unique_id']]
        prior_node["parallel_sequence_id"] = (parallel_id, first_event.attributes[chunk.attr['endpoint']])
    task_create_nodes = deque()
    task_links = deque()
    task_crt_ts = deque()
    task_leave_ts = deque()

    if type(first_event) is Enter and first_event.attributes[chunk.attr['region_type']] in ['initial_task']:
        task_crt_ts.append((first_event.attributes[chunk.attr['unique_id']], first_event.time))

    k = 1
    for event in chain(events, (last_event,)):

        if event.attributes[chunk.attr['region_type']] in ['implicit_task']:
            if type(event) is Enter:
                task_links.append((event.attributes[chunk.attr['encountering_task_id']], event.attributes[chunk.attr['unique_id']]))
                task_crt_ts.append((event.attributes[chunk.attr['unique_id']], event.time))
            continue

        # Add task-leave time
        if type(event) in [Leave] and event.attributes[chunk.attr['region_type']] in ['explicit_task']:
            task_leave_ts.append((get_uid(event, attr=chunk.attr), event.time))

        node = g.add_vertex(event=event)

        if event.attributes[chunk.attr['region_type']] in ['taskgroup']:
            if type(event) is Enter:
                taskgroup_enter_event = event
            elif type(event) is Leave:
                if taskgroup_enter_event is None:
                    raise ValueError("taskgroup-enter event was None")
                node['taskgroup_enter_event'] = taskgroup_enter_event
                taskgroup_enter_event = None

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

        # Add task links and task crt ts
        if (isinstance(event, Enter) and event.attributes[chunk.attr['region_type']] in ['implicit_task']) or (type(event) is ThreadTaskCreate):
            task_links.append((event.attributes[chunk.attr['encountering_task_id']], event.attributes[chunk.attr['unique_id']]))
            task_crt_ts.append((event.attributes[chunk.attr['unique_id']], event.time))

        # For task_create add dummy nodes for easier merging
        if event.attributes[chunk.attr['event_type']] == "task_create":
            node['task_cluster_id'] = (event.attributes[chunk.attr['unique_id']], 'enter')
            dummy_node = g.add_vertex(event=event, task_cluster_id=(event.attributes[chunk.attr['unique_id']], 'leave'))
            task_create_nodes.append(dummy_node)
            continue
        elif len(task_create_nodes) > 0:
            # num_tc_nodes = len(task_create_nodes)
            # for _ in range(num_tc_nodes):
            #     g.add_edge(task_create_nodes.pop(), node)
            task_create_nodes = deque()

        prior_node = node

    if chunk_type == 'explicit_task' and len(events) == 0:
        g.delete_edges([0])

    # Require at least 1 edge between start and end nodes if there are no internal nodes, except for empty explicit task chunks
    if chunk_type != "explicit_task" and len(events) == 0 and g.ecount() == 0:
        g.add_edge(g.vs[0], g.vs[1])

    if verbose and len(events) > 0:
        print(chunk)

    return chunk_type, task_links, task_crt_ts, task_leave_ts, g


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


def append_history(n, f):
    newlines = readline.get_current_history_length()
    readline.set_history_length(1000)
    readline.append_history_file(newlines - n, f)


def descendants_if(node, cond=lambda x: True):
    """Yield all descendants D of node, skipping E & its descendants if cond(E) is False."""
    for child in node.successors():
        if cond(child):
            yield from descendants_if(child, cond=cond)
    yield node.index


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        prog="python3 -m otter",
        description='Convert an Otter OTF2 trace archive to its execution graph representation',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('anchorfile', help='OTF2 anchor file')
    parser.add_argument('-o', '--output', dest='output', help='output file')
    parser.add_argument('-v', '--verbose', action='store_true', dest='verbose', help='print chunks as they are generated')
    parser.add_argument('-i', '--interact', action='store_true', dest='interact', help='drop to an interactive shell upon completion')
    parser.add_argument('-ns', '--no-style', action='store_true', default=False, dest='nostyle', help='do not apply any styling to the graph nodes')
    args = parser.parse_args()

    if args.output is None and not args.interact:
        parser.error("must select at least one of -[o|i]")

    if args.interact:
        print("Otter launched interactively")

    anchorfile = args.anchorfile

    # Map region type to node color
    cmap = defaultdict(lambda: 'white', **{
        'initial_task':      'green',
        'implicit_task':     'fuchsia',
        'explicit_task':     'cyan',
        'parallel':          'yellow',
        'single_executor':   'blue',
        'single_other':      'orange',
        'taskwait':          'red',
        'taskgroup':         'purple',
        'barrier_implicit':  'darkgreen',

        # Workshare regions
        'loop':              'brown',
        'taskloop':          'orange',

        # For colouring by endpoint
        'enter':             'green',
        'leave':             'red'
    })

    cmap_event_type = defaultdict(lambda: 'white', **{
        str(Enter):               'green',
        str(Leave):               'red',
        str(ThreadTaskCreate):    'orange',
    })

    cmap_edge_type = defaultdict(lambda: 'black', **{
        'taskwait':               'red',
        'taskgroup':              'red',
    })

    shapemap = defaultdict(lambda: 'circle', **{
        'initial_task':      'square',
        'implicit_task':     'square',
        'explicit_task':     'square',
        'parallel':          'parallelogram',

        # Sync regions
        'taskwait':          'octagon',
        'taskgroup':         'octagon',
        'barrier_implicit':  'octagon',

        # Workshare regions
        'loop':              'diamond',
        'taskloop':          'diamond',
        'single_executor':   'diamond',
    })

    # Convert event stream into graph chunks
    print(f"loading OTF2 anchor file: {anchorfile}")
    print("generating chunks from event stream...")
    with otf2.reader.open(anchorfile) as tr:
        attr = trace.AttributeLookup(tr.definitions.attributes)
        regions = trace.RegionLookup(tr.definitions.regions)
        results = (process_chunk(chunk, verbose=args.verbose) for chunk in yield_chunks(tr))
        items = zip(*results)
        chunk_types = next(items)
        chain_sort_next = lambda x: sorted(chain(*next(x)), key=lambda t: t[0])
        task_links, task_crt_ts, task_leave_ts = (chain_sort_next(items) for _ in range(3))
        g_list = next(items)

    task_types, *_, task_ids = zip(*[r.name.split() for r in regions.values() if r.region_role == otf2.RegionRole.TASK])
    task_types, task_ids = (zip(*sorted(zip(task_types, map(int, task_ids)), key=lambda t: t[1])))

    # Gather last leave times per explicit task
    task_end_ts = {k: max(u[1] for u in v) for k, v in groupby(task_leave_ts, key=lambda t: t[0])}

    # Task tree showing parent-child links
    task_tree = ig.Graph(edges=task_links, directed=True)
    task_tree.vs['unique_id'] = task_ids
    task_tree.vs['crt_ts'] = [t[1] for t in task_crt_ts]
    task_tree.vs['end_ts'] = [task_end_ts[node.index] if node.index in task_end_ts else None for node in task_tree.vs]
    task_tree.vs['parent_index'] = list(chain((None,), list(zip(*sorted(task_links, key=lambda t: t[1])))[0]))
    task_tree.vs['task_type'] = task_types
    if not args.nostyle:
        task_tree.vs['style'] = 'filled'
        task_tree.vs['color'] = ['red' if v['task_type'] == 'implicit' else 'gray' for v in task_tree.vs]
    tt_layout = task_tree.layout_reingold_tilford()

    # Count chunks by type
    print("graph chunks created:")
    for k, v in Counter(chunk_types).items():
        print(f"  {k:18s} {v:8d}")

    # Collect all chunks
    g = ig.disjoint_union(g_list)
    num_nodes = g.vcount()

    print("{:20s} {:6d}".format("nodes created", num_nodes))

    if 'task_cluster_id' not in g.vs.attribute_names():
        g.vs['task_cluster_id'] = None

    # Collapse by parallel sequence ID
    g.vs['cluster'] = label_clusters(g.vs, lambda v: v['parallel_sequence_id'] is not None, 'parallel_sequence_id')
    nodes_before = num_nodes
    print("contracting by parallel sequence ID")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(attr=attr))
    num_nodes = g.vcount()
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    # Collapse by single-begin/end event
    def is_single_executor(v):
        return type(v['event']) in [Enter, Leave] and v['event'].attributes[attr['region_type']] == 'single_executor'
    g.vs['cluster'] = label_clusters(g.vs, is_single_executor, 'event')
    nodes_before = num_nodes
    print("contracting by single-begin/end event")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(attr=attr))
    num_nodes = g.vcount()
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    # Collapse by (task-ID, endpoint) to get 1 subgraph per task
    for v in g.vs:
        if type(v['event']) in [Enter, Leave] and v['event'].attributes[attr['region_type']] == 'explicit_task':
            v['task_cluster_id'] = (get_uid(v['event'], attr=attr), v['event'].attributes[attr['endpoint']])
    g.vs['cluster'] = label_clusters(g.vs, lambda v: v['task_cluster_id'] is not None, 'task_cluster_id')
    nodes_before = num_nodes
    print("contracting by task ID & endpoint")
    g.contract_vertices(g.vs['cluster'],
                        combine_attrs=attr_handler(events=reject_task_create, tuples=set_tuples, attr=attr))
    num_nodes = g.vcount()
    print("{:20s} {:6d} -> {:6d} ({:6d})".format("nodes updated", nodes_before, num_nodes, num_nodes-nodes_before))

    # Collapse by task ID where there are no links between to combine task nodes with nothing nested within
    def is_empty_task_region(v):
        if v['task_cluster_id'] is None:
            return False
        if type(v['event']) in [Enter, Leave]:
            return (type(v['event']) is Leave and v.indegree() == 0) or \
                   (type(v['event']) is Enter and v.outdegree() == 0)
        if isinstance(v['event'], list) and set(map(type, v['event'])) in [{Enter}, {Leave}]:
            return (set(map(type, v['event'])) == {Leave} and v.indegree() == 0) or \
                   (set(map(type, v['event'])) == {Enter} and v.outdegree() == 0)
    g.vs['cluster'] = label_clusters(g.vs, is_empty_task_region, lambda v: v['task_cluster_id'][0])
    nodes_before = num_nodes
    print("contracting by task ID where there are no nested nodes")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(events=reject_task_create, tuples=set_tuples, attr=attr))
    num_nodes = g.vcount()
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
            e.source_vertex['sync_cluster_id'] = e.target_vertex['sync_cluster_id'] = value
    g.vs['cluster'] = label_clusters(g.vs, lambda v: v['sync_cluster_id'] is not None, 'sync_cluster_id')
    nodes_before = num_nodes
    print("contracting redundant sync-enter/leave node pairs")
    g.contract_vertices(g.vs['cluster'], combine_attrs=attr_handler(tuples=set_tuples, attr=attr))
    num_nodes = g.vcount()
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
        if isinstance(v['endpoint'], set) and len(v['endpoint']) == 1:
            v['endpoint'], = v['endpoint']

    # Apply taskwait synchronisation
    for twnode in g.vs.select(lambda v: v['region_type'] == 'taskwait'):
        tw_encounter_ts = {get_etid(e, attr=attr): e.time for e in twnode['event'] if type(e) is Enter}
        parents = set(task_tree.vs[get_etid(e, attr=attr)] for e in twnode['event'])
        children = [c.index for c in chain(*[p.neighbors(mode='out') for p in parents])
                    if c['crt_ts'] < tw_encounter_ts[c['parent_index']] < c['end_ts']]
        nodes = [v for v in g.vs if v['region_type'] == 'explicit_task'
                                 and get_uid(v['event'], attr=attr) in children
                                 and v['endpoint'] != 'enter']
        ecount = g.ecount()
        g.add_edges([(v.index, twnode.index) for v in nodes])
        g.es[ecount:]['type'] = 'taskwait'

    def event_time_per_task(event):
        """Return the map: encountering task id -> event time for all encountering tasks in the event"""
        if type(event) is list:
            return {get_etid(e, attr=attr): e.time for e in event}
        return {get_etid(event, attr=attr): event.time}

    # Apply taskgroup synchronisation
    for tgnode in g.vs.select(lambda v: v['region_type'] == 'taskgroup' and v['endpoint'] == 'leave'):
        tg_enter_ts = event_time_per_task(tgnode['taskgroup_enter_event'])
        tg_leave_ts = event_time_per_task(tgnode['event'])
        parents = [task_tree.vs[k] for k in tg_enter_ts]
        children = [c for c in chain(*[p.neighbors(mode='out') for p in parents])
                    if tg_enter_ts[c['parent_index']] < c['crt_ts'] < tg_leave_ts[c['parent_index']]]
        descendants = list(chain(*[descendants_if(c, cond=lambda x: x['task_type'] != 'implicit') for c in children]))
        nodes = [v for v in g.vs if v['region_type'] == 'explicit_task'
                                 and get_uid(v['event'], attr=attr) in descendants
                                 and v['endpoint'] != 'enter']
        ecount = g.ecount()
        g.add_edges([(v.index, tgnode.index) for v in nodes])
        g.es[ecount:]['type'] = 'taskgroup'

    # Apply styling if desired
    if not args.nostyle:
        g.vs['color'] = [cmap[v['region_type']] for v in g.vs]
        g.vs['style'] = 'filled'
        g.vs['shape'] = [shapemap[v['region_type']] for v in g.vs]
        g.es['color'] = [cmap_edge_type[e.attributes().get('type', None)] for e in g.es]
    g.vs['label'] = [str(get_uid(v['event'], attr=attr)) if any(s in v['region_type'] for s in ['explicit', 'initial']) else " " for v in g.vs]

    g.simplify(combine_edges='first')

    # Clean up redundant attributes
    for item in ['task_cluster_id', 'parallel_sequence_id', 'cluster', 'sync_cluster_id']:
        if item in g.vs.attribute_names():
            print(f"deleting vertex attribute '{item}'")
            del g.vs[item]

    if args.output:
        print(f"writing graph to '{args.output}'")
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            try:
                g.write(args.output)
            except OSError as oserr:
                print(f"igraph error: {oserr}")
                print(f"failed to write to file '{args.output}'")

    if args.interact:
        import atexit
        import code
        import os
        import readline
        readline.parse_and_bind("tab: complete")

        hfile = os.path.join(os.path.expanduser("~"), ".otter_history")

        try:
            readline.read_history_file(hfile)
            numlines = readline.get_current_history_length()
        except FileNotFoundError:
            open(hfile, 'wb').close()
            numlines = 0

        atexit.register(append_history, numlines, hfile)

        for k, v in locals().items():
            if g is v:
                break

        banner = \
f"""
Graph '{k}' has {g.vcount()} nodes and {g.ecount()} edges

Entering interactive mode, use:
    ig.plot({k}, [target="..."], ...)   to view or plot to file
    {k}.write_*()                       to save a representation of the graph e.g. {k}.write_dot("graph.dot")     
"""
        Console = code.InteractiveConsole(locals=locals())
        Console.interact(banner=banner, exitmsg=f"history saved to {hfile}")
