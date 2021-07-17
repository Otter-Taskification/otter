import otf2
import igraph as ig
import re
import typing as T
from itertools import zip_longest
from collections import deque, defaultdict
from sys import maxsize as MAXINT

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

    def __iter__(self):
        return ((l, e, self.attr) for l, q in self._map.items() for e in q)

    def __call__(self, *args, **kwargs):
        return LocationEventMap(kwargs.get('events', list()), self.attr)

    def __getitem__(self, location):
        if location in self._map:
            return self._map[location]
        else:
            raise KeyError(location)

    def __len__(self):
        return sum([len(k) for k in self.values()])

    def locations(self):
        return sorted(self._map.keys(), key=lambda q: int(q.name.split()[1])) # "Thread x"

    def values(self):
        return self._map.values()

    def items(self):
        return ((l, self[l]) for l in self.locations())

    def append(self, l, e):
        self._map[l].append(e)

    def update(self, other):
        if not isinstance(other, LocationEventMap):
            raise TypeError
        for l, e, a in other:
            self.append(l, e)

    def iter_events(self, location):
        if location in self._map:
            return PushBackIterator(e for e in self._map[location])
        else:
            raise KeyError

    def event_dict(self, event):
        d = dict()
        for attr in event.attributes:
            d[attr.name] = event.attributes[attr]
        return d


class PushBackIterator(object):

    def __init__(self, iterator: T.Iterable):
        self.iter = iterator
        self.saved = deque()

    def __iter__(self):
        return self

    def __next__(self):
        if len(self.saved) > 0:
            return self.saved.pop()
        else:
            return next(self.iter)

    def push(self, item):
        self.saved.append(item)


def event_defines_new_chunk(e: otf2.events._EventMeta, a: AttributeLookup) -> bool:
    return e.attributes.get(a['region_type'], None) in ['parallel', 'explicit_task', 'initial_task', 'single_executor']

def event_defines_chunk_start(e: otf2.events._EventMeta, a: AttributeLookup) -> bool:
    return event_defines_new_chunk(e, a) and isinstance(e, otf2.events.Enter)

def event_defines_chunk_end(e: otf2.events._EventMeta, a: AttributeLookup) -> bool:
    return event_defines_new_chunk(e, a) and isinstance(e, otf2.events.Leave)

def event_defines_parallel_chunk(e: otf2.events._EventMeta, a: AttributeLookup) -> bool:
    return e.attributes.get(a['region_type'], None) == 'parallel'

def filter_task_event(item: tuple) -> bool:
    location, evt, attr = item
    return evt.attributes[attr['event_type']] in ['task_create', 'task_enter', 'task_leave']


class Archive:

    def __init__(self, path: str, verbose: bool = False):
        self.path = path
        with otf2.reader.open(path) as tr:
            self.definitions = tr.definitions
            self.attr = AttributeLookup(self.definitions.attributes)
            print(f"  got {len(self.attr.keys())} attributes")
            self.locations = LocationLookup(self.definitions.locations)
            print(f"  got {len(self.locations.keys())} locations")
            self.regions = RegionLookup(self.definitions.regions)
            print(f"  got {len(self.regions.keys())} regions")
            self.events = LocationEventMap(tr.events, self.attr)
            print(f"  got {len(self.events)} events")
        if verbose:
            print("\n" + ">"*30)
            print("BEGIN TRACE SUMMARY")
            print(">"*30 + "\n")
            print("\nARCHIVE FILE:\n{}".format(path))
            print("\nATTRIBUTES:")
            print(self.attr)
            print("\nLOCATIONS:")
            print(self.locations)
            print("\nREGIONS:")
            print(self.regions)
            print("\nEVENTS:")
            print(self.events)
            print("\n" + "<"*30)
            print("END TRACE SUMMARY")
            print("<"*30 + "\n")

    def yield_trace_chunks(self, is_new_chunk: T.Callable[[otf2.events._EventMeta, AttributeLookup], bool] = event_defines_new_chunk) -> LocationEventMap:
        """
        Break up the self.events lmap into chunks delineated by the events flagged by 'is_new_chunk'
        """
        lmap = self.events()
        stack = deque()
        for l, e, a in self.events:
            if type(e) in [otf2.events.ThreadBegin, otf2.events.ThreadEnd]:
                continue
            if is_new_chunk(e, a):
                # Event marks transition from one chunk to another
                if isinstance(e, otf2.events.Enter):
                    if e.attributes.get(a['region_type'], "") != 'explicit_task':
                        lmap.append(l, e)
                    stack.append(lmap)
                    # New location map for new chunk
                    lmap = self.events(events=[(l,e)])
                elif isinstance(e, otf2.events.Leave):
                    lmap.append(l, e)
                    yield lmap
                    # Continue with enclosing chunk
                    lmap = stack.pop()
                    if e.attributes.get(a['region_type'], "") != 'explicit_task':
                        lmap.append(l, e)
                else:
                    lmap.append(l, e)
            else:
                # Append event to current chunk
                lmap.append(l, e)

    def task_summary(self):
        """
        Summarise all tasks in the trace by their ID
        """
        task_attr = dict()
        for l, e, a in filter(filter_task_event, self.events):
            evt_attr = self.events.event_dict(e)
            task = evt_attr['unique_id']
            if task not in task_attr:
                task_attr[task] = self.events.event_dict(e)
                del task_attr[task]['cpu']
                del task_attr[task]['event_type']
                del task_attr[task]['endpoint']
                del task_attr[task]['prior_task_status']
            if evt_attr['event_type'] == 'task_create':
                task_attr[task]['task_crt_ts'] = e.time
            elif evt_attr['event_type'] == 'task_enter':
                task_attr[task]['task_begin'] = min(e.time, task_attr[task].get('task_begin', MAXINT))
            elif evt_attr['event_type'] == 'task_leave':
                task_attr[task]['task_end'] = max(e.time, task_attr[task].get('task_begin', -MAXINT))
        return task_attr

    def make_task_graph(self):
        """
        Make a graph representing the task creation hierarchy in the trace
        """
        task_attr = self.task_summary()
        task_ids = sorted(list(task_attr.keys()))
        task_parent_ids = [task_attr[k]['parent_task_id'] for k in task_ids]
        num_tasks = len(task_ids)
        tg = ig.Graph(n=num_tasks, directed=True)
        tg.vs['name'] = [str(id) for id in task_ids]
        tg.vs['unique_id'] = task_ids
        tg.vs['label'] = tg.vs['name']
        tg.vs['parent_task_id'] = task_parent_ids

        for v in tg.vs:
            if v['parent_task_id'] in tg.vs['unique_id']:
                tg.add_edge(tg.vs.find(v['parent_task_id']), v)

        return tg
