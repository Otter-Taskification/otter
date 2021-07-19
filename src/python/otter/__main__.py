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


def process_chunk(chunk, color_map: dict=dict()):
    """Return a tuple of chunk type, root nodes, task-create links, taskwait-enter nodes, taskgroup-enter nodes and the chunk's graph"""
    location, = chunk.locations()
    events_iter = iter(chunk[location])
    first_event = next(events_iter)
    chunk_type = first_event.attributes[chunk.attr['region_type']]
    g = ig.Graph(directed=True)
    prior_node = g.add_vertex(event=first_event)
    prior_node['color'] = color_map[first_event.attributes[chunk.attr['region_type']]]
    if chunk_type == 'parallel':
        parallel_id = first_event.attributes[chunk.attr['unique_id']]
        prior_node['cluster_id'] = (parallel_id, 0)
    first_node = prior_node
    task_create_nodes = deque()
    task_links = deque()
    tw_node_pairs = deque()
    tg_node_pairs = deque()
    k = 1
    for event in events_iter:
        node = g.add_vertex(event=event)
        node['color'] = color_map[event.attributes[chunk.attr['region_type']]]
        if chunk_type == 'parallel' and event.attributes[chunk.attr['event_type']] != "task_create":
            node['parallel_cluster_id'] = (parallel_id, k)
            k += 1
        if not(prior_node['event'].attributes[chunk.attr['region_type']] in ['single_executor', 'single_other'] \
               and node['event'].attributes[chunk.attr['region_type']] in ['single_executor', 'single_other'])\
            and not(prior_node['event'].attributes[chunk.attr['region_type']] in ['parallel'] \
               and node['event'].attributes[chunk.attr['region_type']] in ['parallel']):
            g.add_edge(prior_node, node)
        # Add task links
        if event.attributes[chunk.attr['event_type']] in ["task_create", "task_enter"]:
            task_links.append((event.attributes[chunk.attr['encountering_task_id']], event.attributes[chunk.attr['unique_id']]))
        # Add task_create nodes
        if event.attributes[chunk.attr['event_type']] == "task_create":
            task_create_nodes.append(node)
            continue
        elif len(task_create_nodes) > 0:
            num_tc_nodes = len(task_create_nodes)
            for _ in range(num_tc_nodes):
                g.add_edge(task_create_nodes.pop(), node)
        # Save taskwait and taskgroup nodes
        if event.attributes[chunk.attr['region_type']] == "taskwait":
            if event.attributes[chunk.attr['endpoint']] == "enter":
                tw_enter_node = node
            else:
                tw_node_pairs.append((tw_enter_node, node))
        if event.attributes[chunk.attr['region_type']] == "taskgroup":
            if event.attributes[chunk.attr['endpoint']] == "enter":
                tg_enter_node = node
            else:
                tg_node_pairs.append((tg_enter_node, node))
        prior_node = node
    return chunk_type, first_node, task_links, tw_node_pairs, tg_node_pairs, g


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
        'barrier_implicit':  'grey'
    })

    # Convert event stream into graph chunks
    print(f"loading OTF2 anchor file: {anchorfile}")
    print("reading chunks")
    with otf2.reader.open(anchorfile) as tr:
        attr = trace.AttributeLookup(tr.definitions.attributes)
        results = (process_chunk(chunk, cmap) for chunk in yield_chunks(tr))
        items = zip(*(results))
        chunk_types = next(items)
        first_nodes = next(items)
        task_links = next(items)
        tw_node_pairs = next(items)
        tg_node_pairs = next(items)
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

    g.simplify()
    g.vs['color'] = [cmap[v['event'].attributes[attr['region_type']]] for v in g.vs]
    g_layout = g.layout_sugiyama()
