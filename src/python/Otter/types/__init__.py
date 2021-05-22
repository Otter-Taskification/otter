from collections import defaultdict, deque
from typing import Iterable
import otf2
from otf2 import RegionRole
from otf2.events import ThreadBegin, ThreadEnd, Enter, Leave, ThreadTaskCreate
from . import regions, maptypes
import igraph as ig


class Trace:
    """Import an OTF2 trace produced by Otter"""

    def __init__(self, events: Iterable, definitions: Iterable, g: ig.Graph):

        self.definitions = definitions
        self.regions = self.definitions.regions
        self.attributes = {a.name: a for a in self.definitions.attributes}

        self.task_regions = maptypes.TaskIdMap()

        # Parallel and explicit task regions
        self.parallel_regions = {r.name: r for r in self.definitions.regions if r.region_role == RegionRole.PARALLEL}
        self.explicit_task_regions = {r.name: r for r in self.definitions.regions if r.name.startswith("explicit task")}

        # Create nodes for parallel & explicit task regions
        self.parallel_nodes = dict()
        for name in self.parallel_regions.keys():
            unique_id = int(name.split(" ")[-1])
            self.parallel_nodes[unique_id] = {
                'enter': g.add_vertex(name='enter '+name, region_type='parallel', endpoint='enter'),
                'leave': g.add_vertex(name='leave '+name, region_type='parallel', endpoint='leave')}

        self.explicit_task_nodes = dict()
        for name in self.explicit_task_regions.keys():
            unique_id = int(name.split(" ")[-1])
            self.explicit_task_nodes[unique_id] = {
                'enter': g.add_vertex(name='enter '+name, region_type='explicit_task', endpoint='enter'),
                'leave': g.add_vertex(name='leave '+name, region_type='explicit_task', endpoint='leave')}

        self.attr_lookup = {a.name: a for a in self.definitions.attributes}
        self.events = defaultdict(deque)

        for k, (location, event) in enumerate(events):
            if type(event) == ThreadTaskCreate:
                task_name = "{} task {}".format(event.attributes[self.attr_lookup['task_type']], event.attributes[self.attr_lookup['unique_id']])
                event.region = self.explicit_task_regions[task_name]
            self.events[location].append(event)
            if self.get_attribute(event, 'event_type') in ['thread_begin', 'thread_end']:
                continue
            if self.get_attribute(event, 'event_type') == 'task_create' or \
                self.get_attribute(event, 'region_type') in ['initial_task', 'implicit_task'] and \
                    self.get_attribute(event, 'event_type') == 'task_enter':
                self.task_regions[self.get_attribute(event, 'unique_id')] = regions.TaskRegion(
                    self.get_attribute(event, 'unique_id'),
                    self.get_attribute(event, 'region_type'),
                    event,
                    self.task_regions.get(self.get_attribute(event, 'encountering_task_id'), None),
                    self.task_regions.get(self.get_attribute(event, 'encountering_task_id'), None),
                )

        self.num_events = k + 1
        self.num_locations = len(self.definitions.locations)
        self.num_regions = len(self.definitions.regions)
        self.num_strings = len(self.definitions.strings)

    def get_attribute(self, e: otf2.events._EventMeta, key: str):
        return e.attributes[self.attributes[key]]

    def print_events(self):
        for loc, evt_queue in self.events.items():
            print(">>> {} <<<".format(loc.name))
            for evt in evt_queue:
                print("  {:12s} {:16s} {}".format(
                    evt.attributes[self.attr_lookup['endpoint']],
                    evt.attributes[self.attr_lookup['event_type']],
                    evt.region.name + " " + evt.attributes[self.attr_lookup['region_type']] if hasattr(evt, 'region')
                        else evt.attributes[self.attr_lookup['unique_id']]
                            if self.attr_lookup['unique_id'] in evt.attributes
                        else ""
                ))

    def event_defines_subgraph(self, event):
        if type(event) == ThreadTaskCreate:
            return False
        return event.attributes.get(self.attr_lookup['region_type'], None) in ['parallel', 'explicit_task', 'initial_task'] \
           or event.region.name in ['single_executor']

    def gather_events_by_region(self):
        event_map = defaultdict(lambda: defaultdict(deque))
        for location, event_queue in self.events.items():
            region_stack = deque()
            region = None
            for event in event_queue:
                if type(event) in [ThreadBegin, ThreadEnd]:
                    continue
                if self.event_defines_subgraph(event):
                    if type(event) == Enter:
                        if region is not None and event.attributes.get(self.attr_lookup['region_type'], None) != 'explicit_task':
                            event_map[region][location].append(event)
                        region_stack.append(region)
                        region = event.region
                        event_map[region][location].append(event)
                    elif type(event) == Leave:
                        event_map[region][location].append(event)
                        region = region_stack.pop()
                        if region is not None and event.attributes.get(self.attr_lookup['region_type'], None) != 'explicit_task':
                            event_map[region][location].append(event)
                else:
                    event_map[region][location].append(event)
        return event_map
