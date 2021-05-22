import itertools
from collections import deque
from os.path import abspath, basename, dirname
from Otter.types import *


try:
    import igraph as ig
except ModuleNotFoundError as Err:
    print("Failed to import igraph")
    print(Err)
    raise
else:
    print("igraph path: {}".format(dirname(ig.__file__)))


try:
    import otf2
    import _otf2
    from otf2 import events, RegionRole
    from otf2.events import ThreadBegin, ThreadEnd, Enter, Leave
except ModuleNotFoundError as Err:
    print("Failed to import OTF2 module")
    print(Err)
    raise
else:
    print("OTF2 path: {}".format(dirname(otf2.__file__)))


_supported_events = [ThreadBegin, ThreadEnd, Enter, Leave]
_supported_region_roles = [

    # OMP parallel region
    RegionRole.PARALLEL,

    # OMP barriers
    RegionRole.BARRIER,
    RegionRole.IMPLICIT_BARRIER,    # all kinds of implicit barriers
    RegionRole.TASK_WAIT,           # includes TASKGROUP as this isn't separately defined by OTF2

    # OMP workshare regions
    RegionRole.LOOP,                # includes taskloop
    RegionRole.SECTIONS,
    RegionRole.SINGLE,              # includes single_executor and single_other
    RegionRole.WORKSHARE
]


def popleftwhile(predicate, queue):
    while predicate(queue[0]):
        yield queue.popleft()


class ExecutionGraph:
    """Import an OTF2 trace produced by Otter and represent as an execution graph"""

    def __init__(self, trace_path: str):
        self.graph = ig.Graph(directed=True)
        self.path = trace_path
        with otf2.reader.open(trace_path) as tr:
            self.trace = Trace(tr.events, tr.definitions, self.graph)
        summary = f"{'Locations:':12} {self.trace.num_locations}\n"\
            + f"{'Events:':12} {self.trace.num_events}\n"\
            + f"{'Regions:':12} {self.trace.num_regions}\n"\
            + f"{'Strings:':12} {self.trace.num_strings}"
        print(summary)

    def __repr__(self):
        repr = "Trace loaded: {}\n".format(self.path)
        repr += f"{'Locations:':12} {self.trace.num_locations}\n"\
            + f"{'Events:':12} {self.trace.num_events}\n"\
            + f"{'Regions:':12} {self.trace.num_regions}\n"\
            + f"{'Strings:':12} {self.trace.num_strings}"
        return repr

    def event_attribute(self, e: otf2.events._EventMeta, attr_name: str):
        """Lookup the value of an event's attribute by the attribute name"""
        if hasattr(e, 'attributes'):
            return e.attributes.get(self.trace.attr_lookup.get(attr_name, ""), None)
        else:
            return None

    def gather_events_by_region(self):
        self.events_by_region = self.trace.gather_events_by_region()

    def print_events_by_region(self, verbose=False, match=None):
        for region, location_event_map in self.events_by_region.items():
            print(region.name)
            for location, event_queue in location_event_map.items():
                print(location.name)
                for event in event_queue:
                    print("  {}: {} ({} {})".format(
                        type(event),
                        self.event_attribute(event, 'event_type'),
                        self.event_attribute(event, 'region_type'),
                        self.event_attribute(event, 'unique_id')
                    ))
                    # if (type(event)) == ThreadTaskCreate:
                    #     print(event)
                    #     print(event.attributes)
                    if verbose and event.region.name.startswith(match or ""):
                        for attr, value in event.attributes.items():
                            print("    {}: {}".format(attr.name, value))

    def make_subgraphs(self):
        self.parallel_region_nodes = defaultdict(dict)
        self.explicit_task_nodes = defaultdict(dict)
        for region, region_event_map in self.events_by_region.items():
            self.region_to_graph(region, region_event_map)

    def region_to_graph(self, region, location_event_map: dict):
        num_locations = len(location_event_map.keys())
        print("Converting region '{}' ({} locations) to graph".format(region.name, num_locations))
        print("Scanning events across locations: {}".format(", ".join([l.name for l in location_event_map.keys()])))
        if len({len(event_queue) for event_queue in location_event_map.values()}) != 1:
            for location, event_queue in location_event_map.items():
                print("{}: {} events".format(location, len(event_queue)))
            raise ValueError("Different number of events across threads")
        prior_node = None
        for events in zip(*list(location_event_map.values())):
            node_attributes = dict()
            for event in events:
                event_attributes = {attr.name: value for attr, value in event.attributes.items() if event.region.name != "single_other"}
                node_attributes = {**node_attributes, **event_attributes}
            print(": ".join([str({type(e) for e in events}), ", ".join({e.region.name for e in events})]))
            print(", ".join([ ": ".join([str(type(e)), e.region.name]) for e in events]))
            print("NODE ATTRIBUTES: {}".format(node_attributes))
            if node_attributes.get('region_type', '') == 'implicit_task':
                continue
            if node_attributes.get('region_type', '') == 'parallel':
                parallel_name = events[0].region.name
                if parallel_name in self.parallel_region_nodes and node_attributes['endpoint'] in self.parallel_region_nodes[parallel_name]:
                    n = self.parallel_region_nodes[parallel_name][node_attributes['endpoint']]
                else:
                    n = self.graph.add_vertex(name='{} {}'.format(node_attributes['endpoint'], parallel_name), **node_attributes)
                    self.parallel_region_nodes[parallel_name][node_attributes['endpoint']] = n
            elif node_attributes.get('region_type', '') == 'explicit_task':
                task_name = events[0].region.name
                if task_name in self.explicit_task_nodes and node_attributes['endpoint'] in self.explicit_task_nodes[task_name]:
                    n = self.explicit_task_nodes[task_name][node_attributes['endpoint']]
                else:
                    n = self.graph.add_vertex(name='{} {}'.format(node_attributes['endpoint'], task_name), **node_attributes)
                    self.explicit_task_nodes[task_name][node_attributes['endpoint']] = n
            else:
                n = self.graph.add_vertex(**node_attributes)
            if prior_node is not None:
                print("Adding edge between nodes:")
                print(prior_node)
                print(n)
                self.graph.add_edge(prior_node, n)
            prior_node = n
