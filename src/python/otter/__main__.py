import argparse
import re
import warnings
import igraph as ig
import otf2
from itertools import chain, zip_longest, cycle
from collections import defaultdict, deque
from sys import stdout
from time import time
from otf2.events import Enter, Leave
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


# if __name__ == "__main__":
#
#     dt = 0.2
#     symbols = "/-\\|"
#     symbiter = cycle(symbols)
#
#     g = ig.Graph(directed=True)
#
#     parser = argparse.ArgumentParser(
#         description='Convert an Otter OTF2 trace archive to its execution graph representation',
#         formatter_class=argparse.ArgumentDefaultsHelpFormatter)
#
#     parser.add_argument('anchorfile', help='OTF2 anchor file')
#     args = parser.parse_args()
#
#     print(f"loading OTF2 anchor file: {args.anchorfile}")
#     tr = trace.Archive(args.anchorfile, verbose=False)
#
#     # Summary data for all tasks in the trace:
#     print("summarising task data")
#     task_attr = tr.task_summary()
#
#     # Construct the task tree
#     print("creating task tree")
#     task_graph = tr.make_task_graph()
#
#     # Define some formatting arguments according to region_type attribute
#     vertex_default_format = {'shape': 'circle', 'color': 'red'}
#     vertex_type_format = {'parallel': {'shape': 'circle', 'color': 'darkorange'},
#                           'initial_task': {'shape': 'rectangle', 'color': 'purple'},
#                           'explicit_task': {'shape': 'rectangle', 'color': 'darkcyan'},
#                           'loop': {'shape': 'triangle-up', 'color': 'coral'},
#                           'taskloop': {'shape': 'triangle-up', 'color': 'salmon'},
#                           'taskgroup': {'shape': 'triangle-down', 'color': 'green'},
#                           'taskwait': {'shape': 'triangle-down', 'color': 'deeppink'},
#                           'single_executor': {'shape': 'circle', 'color': 'skyblue'},
#                           'barrier_implicit': {'shape': 'circle', 'color': 'turquoise'}}
#
#     # For all parallel, initial and explicit task regions (i.e. regions which are globally defined), create
#     # enter and leave nodes
#     print("creating nodes for globally defined regions")
#     for r in filter(ft.is_global_region, tr.definitions.regions):
#         uid = int(re.search(r'\d+', r.name)[0])
#         if "parallel" in r.name.lower():
#             region_type = "parallel"
#         elif "explicit" in r.name.lower():
#             region_type = "explicit_task"
#         elif "initial" in r.name.lower():
#             region_type = "initial_task"
#         else:
#             print("unhandled region: {} ({})".format(r.name, r.region_role))
#             raise ValueError
#         n1 = g.add_vertex(region_type=region_type, unique_id=uid, endpoint="enter")
#         n2 = g.add_vertex(region_type=region_type, unique_id=uid, endpoint="leave")
#
#     # Handle all non-parallel regions first - those regions generated by events from a single thread
#     print("adding non-parallel region events to graph")
#     t0 = time()
#     for n, chunk in enumerate(filter(lambda c: not ft.is_parallel_context(c), tr.yield_trace_chunks())):
#
#         now = time()
#         if now - t0 >= dt:
#             stdout.write("\r  {} chunk {}".format(next(symbiter), n))
#             stdout.flush()
#             t0 = now
#
#         # Expect each chunk to contain one location and a sequence of its events - anything else is an error
#         try:
#             location, = chunk.locations()
#         except ValueError:
#             print("Misaligned non-parallel chunk:")
#             print(chunk)
#             raise
#         events = iter(chunk[location])
#         final_event = chunk[location][-1]
#         num_events = len(chunk[location])
#
#         # Get first event in chunk
#         prior_event = next(events)
#         prior_event_attr = chunk.event_dict(prior_event)
#
#         # Create stack of events to match enter & leave event pairs
#         event_stack = deque()
#         event_stack.append(prior_event)
#
#         # Lookup nodes previously created for non-parallel global regions
#         # Otherwise, create a new node
#         if ft.is_global_region(prior_event.region):
#             # initial- & explicit-task event-chunks
#             prior_node, = g.vs.select(lambda v: v['region_type'] == prior_event_attr['region_type']
#                                                 and v['unique_id'] == prior_event_attr['unique_id']
#                                                 and v['endpoint'] == prior_event_attr['endpoint'])
#         elif prior_event_attr['region_type'] == 'single_executor':
#             # single-executor chunks
#             prior_node = g.add_vertex()
#         else:
#             raise ValueError(f"unexpected region type: {prior_event.region.name} ({prior_event.region.region_role})")
#
#         # Apply event attributes to the node
#         prior_node.update_attributes(prior_event_attr)
#         prior_node['event'] = prior_event
#
#         # Empty list for tracking task-create nodes
#         task_create_nodes = list()
#
#         for event in events:
#
#             # Event as a dict of attributes - does not include event time!
#             event_attr = chunk.event_dict(event)
#
#             # Push to/pop from event stack according to endpoint
#             if event_attr['endpoint'] == 'enter':
#                 event_stack.append(event)
#                 enter_event = None
#             elif event_attr['endpoint'] == 'leave':
#                 enter_event = event_stack.pop()
#                 enter_event_attr = tr.events.event_dict(enter_event)
#
#             # Lookup nodes previously created for global regions (including parallel regions)
#             if event_attr['region_type'] in ['parallel', 'initial_task', 'explicit_task'] \
#                     and event_attr['endpoint'] in ['enter', 'leave']:
#                 node, = g.vs.select(lambda v: v['region_type'] == event_attr['region_type']
#                                               and v['unique_id'] == event_attr['unique_id']
#                                               and v['endpoint'] == event_attr['endpoint'])
#
#             # Store taskwait-enter time keyed by encountering task for compatability with later parallel region code
#             elif event_attr['region_type'] == 'taskwait' and event_attr['endpoint'] == 'enter':
#                 event_attr['taskwait_enter_time'] = {event_attr['encountering_task_id']: event.time}
#                 event_attr['encountering_task_id_list'] = [event_attr['encountering_task_id'],]
#                 node = g.add_vertex()
#
#             # Store taskgroup event times for compatability with later parallel region code
#             elif event_attr['region_type'] == 'taskgroup':
#                 event_attr['taskgroup_event_time'] = {event_attr['encountering_task_id']: event.time}
#                 event_attr['encountering_task_id_list'] = [event_attr['encountering_task_id'],]
#                 # Need to know the time a taskgroup region began & ended for task synchronisation
#                 if event_attr['endpoint'] == 'leave':
#                     event_attr['taskgroup_ts'] = {event_attr['encountering_task_id']: (enter_event.time, event.time)}
#                 node = g.add_vertex()
#
#             # Shouldn't find nested single-executor regions
#             elif event_attr['region_type'] in ['single_executor'] and event is not final_event:
#                 raise ValueError("Unexpected single-executor event")
#
#             else:
#                 node = g.add_vertex()
#
#             # Apply event attributes to the node
#             node.update_attributes(event_attr)
#             node['event'] = event
#
#             # Add edge between nodes unless they are the start & end nodes of another chunk
#             # (but if there are only enter & leave events, create an edge regardless)
#             if (not (trace.event_defines_chunk_start(prior_event, tr.attr) and trace.event_defines_chunk_end(event, tr.attr))) \
#                     or (num_events == 2)\
#                     or (num_events == 2 + len(task_create_nodes)):
#                 e = g.add_edge(prior_node, node)
#
#             # Following one or more task-create events, join the list of task-create nodes to the next node
#             if event_attr['event_type'] != 'task_create' and len(task_create_nodes) > 0:
#                 task_create_nodes = list()
#
#             # Don't update prior node + event if current node is a task-create node so that all task-create nodes are
#             # joined to the same predecessor node
#             if event_attr['event_type'] == 'task_create':
#                 task_create_nodes.append(node)
#             else:
#                 prior_event, prior_node = event, node
#     print("  {} chunk {}".format(next(symbiter), n))
#
#     # Gather parallel chunks into 1 LocationEventMap per parallel region (mapping each participating thread to its
#     # events in that region)
#     parallel_region_lmap_lookup = defaultdict(lambda: trace.LocationEventMap(list(), tr.attr))
#     for chunk in filter(ft.is_parallel_context, tr.yield_trace_chunks()):
#
#         # Expect exactly 1 bounding region - any more is an error
#         bounding_rgns = [(chunk[loc][0].region, chunk[loc][-1].region) for loc in chunk.locations()]
#         rgn, = set(chain(*bounding_rgns))
#
#         # Update corresponding lmap with this chunk of events
#         parallel_region_lmap_lookup[rgn].update(chunk)
#
#     # Next handle the parallel regions - these are more complicated as each thread has its own view of the events
#     print("adding parallel region events to graph")
#     for n, (enclosing_parallel_region, lmap) in enumerate(parallel_region_lmap_lookup.items()):
#
#         locations = lmap.locations()
#         event_iterators = [lmap.iter_events(l) for l in locations]
#
#         # First event of each location that visited this region
#         prior_events = [next(it) for it in event_iterators]
#
#         # Event stack to match up sequences of enter & leave events
#         event_stack = deque()
#         event_stack.append(prior_events)
#
#         # List of event attribute dicts
#         prior_events_attr = list(map(lmap.event_dict, prior_events))
#
#         endpoints = {d['endpoint'] for d in prior_events_attr}
#         region_types = {d['region_type'] for d in prior_events_attr}
#         event_types = {d['event_type'] for d in prior_events_attr}
#
#         # Expect exactly 1 value in each set, anything else is an error
#         try:
#             (endpoint,), (region_type,), (event_type,) = endpoints, region_types, event_types
#         except ValueError as V:
#             print("*** ERROR: Misaligned parallel location map: ***")
#             print(f"{endpoints=}")
#             print(f"{region_types=}")
#             print(f"{event_types=}")
#             raise
#
#         # Unique attribute keys
#         attr_keys = set(chain(*[d.keys() for d in prior_events_attr]))
#         event_attr = {k: ", ".join(set(str(a[k]) for a in prior_events_attr)) for k in attr_keys}
#
#         # Look up the parallel nodes that were created earlier
#         parallel_begin_node, = g.vs.select(lambda v: v['region_type'] == event_attr['region_type']
#                                                      and v['unique_id'] == int(event_attr['unique_id'])
#                                                      and v['endpoint'] == 'enter')
#         parallel_end_node, = g.vs.select(lambda v: v['region_type'] == event_attr['region_type']
#                                                    and v['unique_id'] == parallel_begin_node['unique_id']
#                                                    and v['endpoint'] == 'leave')
#
#         prior_nodes = [parallel_begin_node,]
#
#         # Process the remaining events in this chunk
#         for event_num, next_events in enumerate(zip_longest(*event_iterators)):
#
#             # Each event as a dictionary of its attributes
#             next_events_attr = list(map(lmap.event_dict, next_events))
#             endpoints = {d['endpoint'] for d in next_events_attr}
#             region_types = {d['region_type'] for d in next_events_attr}
#             event_types = {d['event_type'] for d in next_events_attr}
#
#             # For single constructs, interested only in the single-executor event & its associated chunk
#             if region_types == {'single_executor', 'single_other'}:
#                 region_types = {'single_executor'}
#
#             # If any event is of type task_create, consume all task_create events for those locations which recorded
#             # them, then continue with the next event per location
#
#             # Expect exactly one endpoint & region type among the current events
#             try:
#                 (endpoint,), (region_type,), (event_type,) = endpoints, region_types, event_types
#             except ValueError as V:
#                 print("*** ERROR: Misaligned parallel location map: ***")
#                 print(f"{endpoints=}")
#                 print(f"{region_types=}")
#                 print(f"{event_types=}")
#                 raise
#
#             # Don't create nodes for implicit tasks
#             if region_type == 'implicit_task':
#                 continue
#
#             # Set of attribute names
#             attr_keys = set(chain(*[a.keys() for a in next_events_attr]))
#
#             # Combine multiple event attributes into 1 dict of attributes
#             event_attr = {k: ", ".join(set(str(a[k]) for a in next_events_attr)) for k in attr_keys}
#             event_attr['region_type'] = region_type
#
#             # Track matching enter & leave events using a stack of events
#             if endpoint == 'enter':
#                 event_stack.append(next_events)
#                 enter_events = list()
#                 enter_events_attr = list()
#             elif endpoint == 'leave':
#                 enter_events = event_stack.pop()
#                 enter_events_attr = list(map(lmap.event_dict, enter_events))
#                 event_attr['ts_interval'] = {e.attributes[tr.attr['encountering_task_id']]: (e.time, l.time) for e,l in zip(enter_events, next_events)}
#
#             # For taskwait nodes, add dict to lookup taskwait-enter time by encountering task ID
#             if event_attr['region_type'] == 'taskwait' and event_attr['endpoint'] == 'enter':
#                 event_attr['taskwait_enter_time'] = {e.attributes[tr.attr['encountering_task_id']]: e.time for e in next_events}
#                 event_attr['encountering_task_id_list'] = [e['encountering_task_id'] for e in next_events_attr]
#
#             # For taskgroup regions, record enter and leave event times by encountering task ID
#             elif event_attr['region_type'] == 'taskgroup':
#                 event_attr['taskgroup_event_time'] = {e.attributes[tr.attr['encountering_task_id']]: e.time for e in next_events}
#                 event_attr['encountering_task_id_list'] = [e['encountering_task_id'] for e in next_events_attr]
#                 if event_attr['endpoint'] == 'leave':
#                     event_attr['taskgroup_ts'] = event_attr['ts_interval']
#
#             # Set of regions of these events
#             try:
#                 regions = {e.region for e in next_events}
#             except AttributeError:
#                 regions = {None, }
#
#             # Initialise empty list of nodes
#             nodes = list()
#
#             # These region types explicitly not currently supported
#             if region_type in ['target_task', 'teams']:
#                 raise ValueError("region type not yet supported: {}".format(region_type))
#
#             # Nested parallel region: lookup existing nodes
#             elif region_type == 'parallel' and enclosing_parallel_region not in regions:
#                 nodes = g.vs.select(lambda v: v['region_type'] == region_type
#                                               and v['unique_id'] in [e['unique_id'] for e in next_events_attr]
#                                               and v['endpoint'] == endpoint)
#
#             # If a task-create occurs in a parallel region, all threads created tasks - add task-create nodes
#             elif region_type == 'explicit_task':
#                 if event_type == 'task_create':
#                     # Create nodes for these events
#                     for evt, attr in zip(next_events, next_events_attr):
#                         nodes.append(g.add_vertex(**attr, event=evt))
#                     # Consume all following task-create events in each event iterator:
#                     print("  consuming task-create events")
#                     for n, event_iterator in enumerate(event_iterators):
#                         for count, evt in enumerate(event_iterator):
#                             attr = lmap.event_dict(evt)
#                             if attr['event_type'] == 'task_create':
#                                 nodes.append(g.add_vertex(**attr, event=evt))
#                             else:
#                                 event_iterator.push(evt)
#                                 print(f"    consumed {count} events from {locations[n].name}")
#                                 break
#                 else:
#                     print(f"Unexpected explicit task event type: {event_type=}")
#                     raise ValueError
#
#             # Single-executor node should have been created already & updated with event attributes, so lookup node
#             # from event
#             elif region_type == 'single_executor':
#                 nodes = g.vs.select(lambda v: v['event'] in next_events)
#                 if len(nodes) != 1:
#                     raise IndexError("invalid number of nodes returned")
#
#             # End of enclosing parallel region
#             elif region_type == 'parallel' and endpoint == 'leave' and enclosing_parallel_region in regions:
#                 nodes = [parallel_end_node, ]
#
#             # Create new node by combining events into a single representation
#             else:
#                 nodes = [g.add_vertex(**event_attr), ]
#
#             # Add edges between nodes unless they are the start & end nodes of another chunk
#             if not (    any([n['endpoint'] == 'enter' for n in prior_nodes])
#                     and any([n['endpoint'] == 'leave' for n in nodes])
#                     and any([n['region_type'] in ['parallel', 'explicit_task', 'initial_task', 'single_executor'] for n in prior_nodes])
#                     and any([n['region_type'] in ['parallel', 'explicit_task', 'initial_task', 'single_executor'] for n in nodes])):
#                 if len(prior_nodes) == 1 and len(nodes) == 1:
#                     # 1-1
#                     e = g.add_edge(*prior_nodes, *nodes)
#                 elif len(prior_nodes) == 1 and len(nodes) > 1:
#                     # 1-many
#                     for n in nodes:
#                         e = g.add_edge(*prior_nodes, n)
#                 elif len(nodes) == 1 and len(prior_nodes) > 1:
#                     # many-1
#                     for p in prior_nodes:
#                         e = g.add_edge(p, *nodes)
#                 elif len(prior_nodes) > 1 and len(nodes) > 1:
#                     # many-many
#                     sort_by_id = lambda v: v['unique_id']
#                     prior_sorted = sorted(prior_nodes, key=sort_by_id)
#                     nodes_sorted = sorted(nodes, key=sort_by_id)
#                     for p, n in zip(prior_sorted, nodes_sorted):
#                         e = g.add_edge(p, n)
#
#             # Don't update prior node + event if current node is a task-create node
#             if event_attr['event_type'] != 'task_create':
#                 prior_events = next_events
#                 prior_events_attr = next_events_attr
#                 prior_nodes = nodes
#
#
#     # For each taskwait node, get the task-create nodes of tasks created before the taskwait-enter event
#     print("processing taskwait nodes")
#     for twnode in g.vs.select(lambda v: v['region_type'] == 'taskwait' and v['endpoint'] == 'enter'):
#
#         # Get the tasks with the same parent task ID
#         parent_tasks = twnode['encountering_task_id_list']
#         child_tasks = set(chain(*task_graph.neighborhood(parent_tasks, mode='out', order=1))) - set(parent_tasks)
#
#         # Look up the task-enter nodes with the same encountering task as the taskwait region, created before the
#         # taskwait was encountered
#         for tcnode in g.vs.select(lambda v: v['event_type']=='task_create' and
#                 v['unique_id'] in child_tasks and
#                 v['event'].time < twnode['taskwait_enter_time'][v['parent_task_id']] and
#                 v.attributes().get('taskwaitnode', None) is None):
#             tcnode['taskwaitnode'] = twnode
#
#             # Get the corresponding task-leave node (assuming only one):
#             tlnode, = g.vs.select(lambda v: v['event_type']=='task_leave' and v['unique_id']==tcnode['unique_id'])
#
#             # If there is already an edge between these nodes, retrieve it. Otherwise create it
#             try:
#                 eid = g.get_eid(tlnode.index, twnode.index)
#                 e = g.es[eid]
#                 e['color'] = 'red'
#             except ig._igraph.InternalError:
#                 e = g.add_edge(tlnode, twnode)
#             e['color'] = 'red'
#
#     print("processing taskgroup nodes")
#     for tgnode in g.vs.select(lambda v: v['region_type']=='taskgroup' and v['endpoint']=='leave'):
#         parent_tasks = tgnode['encountering_task_id_list']
#
#         # Get child tasks of the task that encountered the taskgroup region, that were created during this taskgroup
#         child_task_node_iter = g.vs.select(lambda v: v['event_type']=='task_create' and
#             v['encountering_task_id'] in parent_tasks and
#             # child task must be created during taskgroup region
#             v['event'].time > tgnode['taskgroup_ts'][v['encountering_task_id']][0] and
#             v['event'].time < tgnode['taskgroup_ts'][v['encountering_task_id']][1])
#         child_tasks = [node['unique_id'] for node in child_task_node_iter]
#         descendant_tasks = set(chain(*[task_graph.dfs(t)[0] for t in child_tasks]))
#
#         for tcnode in g.vs.select(lambda v: v['event_type']=='task_create' and v['unique_id'] in descendant_tasks):
#             tcnode['taskgroupnode'] = tgnode
#             g.add_edge(tcnode, tgnode)
#
#     # Replace each explicit-task-create node with the corresponding explicit-task-enter node (assuming only 1 such node)
#     # Replace the outedges of the explicit-task-create node
#     to_delete = list()
#     print("substituting task-create nodes for task-enter nodes")
#     for taskcreatenode in g.vs.select(lambda v: v['region_type'] == 'explicit_task' and v['event_type'] == 'task_create'):
#         inedge, = taskcreatenode.in_edges()
#         outedges = taskcreatenode.out_edges()
#         task_enter_node, = g.vs.select(lambda v: v['region_type'] == 'explicit_task' and
#                                                  v['endpoint'] == 'enter' and v['unique_id'] == taskcreatenode['unique_id'])
#         task_leave_node, = g.vs.select(lambda v: v['region_type'] == 'explicit_task' and
#                                                  v['endpoint'] == 'leave' and v['unique_id'] == taskcreatenode['unique_id'])
#         g.add_edge(inedge.source_vertex, task_enter_node)
#         for e in outedges:
#             g.add_edge(task_leave_node, e.target_vertex, color=e['color'])
#         to_delete.append(taskcreatenode)
#         taskcreatenode['color'] = 'red'
#
#     g.delete_vertices(to_delete)
#
#     g = g.simplify(combine_edges="first")
#
#     # Reduce redundant pairs of nodes by looping over edges and deleting the destination nodes where the region_type matches and the endpoints are enter+leave
#     to_delete = list()
#     print("reducing redundant pairs of nodes")
#     for edge in g.es.select(
#             lambda e: e.source_vertex['region_type'] == e.target_vertex['region_type'] and e.source_vertex[
#                 'endpoint'] == 'enter' and e.target_vertex['endpoint'] == 'leave'):
#         if edge.source_vertex.outdegree() == 1:
#             to_delete.append(edge.target_vertex)
#             # Replace edges leaving the target vertex
#             for outedge in edge.target_vertex.out_edges():
#                 g.add_edge(edge.source_vertex, outedge.target_vertex, color=outedge.attributes().get('color', 'black'))
#
#     g.delete_vertices(to_delete)
#
#     # Remove redundant edges between task-enter and task-leave nodes where there are any other edges present
#     to_delete = list()
#     print("removing redundant task edges")
#     for edge in g.es.select(lambda e: e.source_vertex['region_type'] == 'explicit_task'
#             and e.target_vertex['region_type'] == 'explicit_task'
#             and e.source_vertex['unique_id'] == e.target_vertex['unique_id']
#             and e.source_vertex['endpoint'] == 'enter'
#             and e.target_vertex['endpoint'] == 'leave'
#             and e.target_vertex.indegree() > 1):
#         to_delete.append(edge)
#
#     g.delete_edges(to_delete)
#
#     # Apply formatting based on region_type attribute:
#     # Label nodes with unique id
#     for v in g.vs:
#         v.update_attributes(vertex_type_format.get(v['region_type'], vertex_default_format))
#         v.update_attributes(label=v['unique_id'])
#
#     # plot_graph(g, vertex_size=35, bbox=(1200,800), margin=80, target="graph.svg")
#     # graph_to_dot(g, target="graph.dot")

def yield_chunks(tr, g: ig.Graph):
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


def process_chunk(chunk, n=None):
    location, = chunk.locations()
    first, *_, last = chunk[location]
    # if n is None:
    #     print(f"  {location.name}: {first.attributes[chunk.attr['event_type']]}")
    # else:
    #     print(f"{n}  {location.name}: {first.attributes[chunk.attr['event_type']]:<16s} {last.attributes[chunk.attr['event_type']]:<16s} {first.attributes[chunk.attr['region_type']]:<16s}")
    return first.attributes[chunk.attr['region_type']]

def print_count(chunk_counter, k):
    stdout.write("\033[F\033[K" * 4)
    stdout.flush()
    for m, name in enumerate(['initial_task', 'parallel', 'single_executor', 'explicit_task']):
        print(f"  {name:18s} {chunk_counter[name]:8d}")

if __name__ == "__main__":

    g = ig.Graph(directed=True)

    parser = argparse.ArgumentParser(
        description='Convert an Otter OTF2 trace archive to its execution graph representation',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('anchorfile', help='OTF2 anchor file')
    args = parser.parse_args()

    print(f"loading OTF2 anchor file: {args.anchorfile}")
    print("reading chunks")
    for m, name in enumerate(['initial_task', 'parallel', 'single_executor', 'explicit_task']):
        print(f"  {name:18s} {0:8d}")
    with otf2.reader.open(args.anchorfile) as tr:
        chunk_counter = defaultdict(int)
        for k, chunk in enumerate(yield_chunks(tr, g)):
            chunk_counter[process_chunk(chunk, k)] += 1
            if k % 1000 == 999:
                print_count(chunk_counter, k)
        print_count(chunk_counter, k)