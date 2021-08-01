import re
import typing as T
import igraph as ig
import otf2
from itertools import chain
from collections import defaultdict, deque
from otf2.events import Enter, Leave, ThreadTaskCreate
from otter.helpers import get_uid

class DefinitionLookup:

    def __init__(self, registry: otf2.registry._RefRegistry):
        self._lookup = dict()
        for d in registry:
            if d.name in self._lookup:
                raise KeyError("{} already present".format(d.name))
            self._lookup[d.name] = d

    def __getitem__(self, name):
        if name in self._lookup:
            return self._lookup[name]
        else:
            raise AttributeError(name)

    def __iter__(self):
        return ((k,v) for k, v in self._lookup.items())

    def keys(self):
        return self._lookup.keys()

    def values(self):
        return self._lookup.values()

    def items(self):
        return self.__iter__()


class AttributeLookup(DefinitionLookup):

    def __init__(self, attributes: otf2.registry._RefRegistry):
        if not isinstance(attributes[0], otf2.definitions.Attribute):
            raise TypeError(type(attributes[0]))
        super().__init__(attributes)

    def __repr__(self):
        s = "{:24s} {:12s} {}\n".format("Name", "Type", "Description")
        format = lambda k, v: "{:24s} {:12s} {}".format(k, str(v.type).split(".")[1], v.description)
        return s+"\n".join([format(k, v) for k,v in self._lookup.items()])


class LocationLookup(DefinitionLookup):

    def __init__(self, locations: otf2.registry._RefRegistry):
        if not isinstance(locations[0], otf2.definitions.Location):
            raise TypeError(type(locations[0]))
        super().__init__(locations)

    def __repr__(self):
        s = "{:12s} {:12s} {:12s} {}\n".format("Group", "Name", "Type", "Events")
        format = lambda v: "{:12s} {:12s} {:12s} {}".format(v.group.name, v.name, str(v.type).split(".")[1], v.number_of_events)
        return s+"\n".join([format(v) for k,v in self._lookup.items()])


class RegionLookup(DefinitionLookup):

    def __init__(self, regions: otf2.registry._RefRegistry):
        self._lookup = dict()
        for r in regions:
            ref = int(re.search(r'\d+', repr(r))[0])
            if ref in self._lookup:
                raise KeyError("{} already present".format(ref))
            self._lookup[ref] = r

    def __repr__(self):
        minref, maxref = min(self._lookup.keys()), max(self._lookup.keys())
        s = "{:3s} {:18s} {}\n".format("Ref", "Name", "Role")
        format_item = lambda l, k: "{:3d} {:18s} {}".format(k, l[k].name, str(l[k].region_role).split(".")[1])
        return s + "\n".join([format_item(self._lookup, i) for i in range(minref, maxref+1)])


class LocationEventMap:
    """
    Behaves like a collection of dicts, each of which maps a location (thread) onto some sequence of events recorded
    by that location
    """

    def __init__(self, events: T.Iterable, attr: AttributeLookup):
        self._map = defaultdict(deque)
        self.attr = attr
        for l, e in events:
            self._map[l].append(e)

    def __repr__(self):
        s = ""
        for l, q in self._map.items():
            s += l.name + "\n"
            s += "  {:18s} {:10s} {:20s} {:20s} {:18s} {}\n".format("Time", "Endpoint", "Region Type", "Event Type", "Region ID/Name", "CPU")
            for e in q:
                s += "  {:<18d} {:10s} {:20s} {:20s} {:18s} {}\n".format(
                    e.time,
                    e.attributes[self.attr['endpoint']],
                    e.attributes.get(self.attr['region_type'], ""),
                    e.attributes[self.attr['event_type']],
                    str(e.attributes[self.attr['unique_id']]) if self.attr['unique_id'] in e.attributes else e.region.name,
                    e.attributes[self.attr['cpu']])
        return s

    def __getitem__(self, location):
        if location in self._map:
            return self._map[location]
        else:
            raise KeyError(location)

    def locations(self):
        return sorted(self._map.keys(), key=lambda q: int(q.name.split()[1])) # "Thread x"

    def items(self):
        return ((l, self[l]) for l in self.locations())

    def append(self, l, e):
        self._map[l].append(e)


def yield_chunks(tr):
    attr = AttributeLookup(tr.definitions.attributes)
    lmap_dict = defaultdict(lambda : LocationEventMap(list(), attr))
    stack_dict = defaultdict(deque)
    for location, event in tr.events:
        if type(event) in [otf2.events.ThreadBegin, otf2.events.ThreadEnd]:
            continue
        if event_defines_new_chunk(event, attr):
            # Event marks transition from one chunk to another
            if isinstance(event, Enter):
                if event.attributes.get(attr['region_type'], "") != 'explicit_task':
                    lmap_dict[location].append(location, event)
                stack_dict[location].append(lmap_dict[location])
                # New location map for new chunk
                lmap_dict[location] = LocationEventMap([(location, event)], attr)
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


def event_defines_new_chunk(e: otf2.events._EventMeta, a: AttributeLookup) -> bool:
    return e.attributes.get(a['region_type'], None) in ['parallel', 'explicit_task', 'initial_task', 'single_executor']


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
