import itertools
from collections import deque


try:
    import otf2
    import _otf2
    from otf2 import events, RegionRole
    from otf2.events import ThreadBegin, ThreadEnd, Enter, Leave
except ModuleNotFoundError as Err:
    print("Failed to import OTF2 module")
    print(Err)
else:
    from os.path import abspath, basename, dirname
    print("OTF2 path: {}".format(dirname(otf2.__file__)))
from Otter.types import *


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
        self.graph = Graph()
        self.events_padded = defaultdict(deque)
        with otf2.reader.open(trace_path) as tr:
            self.trace = Trace(tr.events, tr.definitions)
        msg = f"{'Locations:':12} {self.trace.num_locations}\n"\
            + f"{'Events:':12} {self.trace.num_events}\n"\
            + f"{'Regions:':12} {self.trace.num_regions}\n"\
            + f"{'Strings:':12} {self.trace.num_strings}"
        print(msg)

    def event_attribute(self, e: otf2.events._EventMeta, attr_name: str):
        """Lookup the value of an event's attribute by the attribute name"""
        if hasattr(e, 'attributes'):
            return e.attributes[self.trace.attr_lookup[attr_name]]
        else:
            return None

    def pad_events(self):
        """Pad each location's event queue so that collective events between locations are aligned across queues"""

        # Expect all locations to have the same type of initial event (ThreadBegin)
        first_type , = {type(event[0]) for event in self.trace.events.values()}
        if not first_type is ThreadBegin:
            raise TypeError(f"Invalid initial event type: {first_type}")

        # While any location still has events to consume
        while max(map(len, self.trace.events.values())) != 0:

            # type of next event for each location
            next_events = {location: event.popleft() for location, event in self.trace.events.items()}
            next_types  = set(map(type, next_events.values()))

            # Append popped events before scanning ahead
            for loc, event in next_events.items():
                self.events_padded[loc].append(event)

            if len(next_types) == 1:

                # Get the event type encountered
                next_type = next_types.pop()

                # Raise error if unsupported event type encountered
                if next_type not in _supported_events:
                    raise TypeError(f"Unsupported event type: {next_type}")

                # Handle region-begin
                if next_type is Enter:

                    # Detect the region type(s) entered
                    region_roles = {e.region.region_role for e in next_events.values()}

                    if not all([role in _supported_region_roles for role in region_roles]):
                        raise TypeError(f"Unsupported region role in: {region_roles}")

                    if len(region_roles) != 1:
                        raise TypeError(f"Multiple region roles encountered for region-enter event: {region_roles}")

                    # We have region-begin events for all threads, all of same region type
                    region_role = region_roles.pop()

                    if region_role == RegionRole.SINGLE:

                        # Exactly one of the threads should have event.attributes['workshare_type'] == 'single_executor'
                        num_executors = len([e for e in next_events.values() if
                                             self.event_attribute(e, 'workshare_type') == 'single_executor'])

                        if num_executors != 1:
                            raise ValueError(f"Invalid number of single executors: {num_executors}")

                        # Lambda that returns true for required single-end event:
                        filter_single_end = lambda e: not (type(e) == Leave \
                                                           and e.region.region_role == RegionRole.SINGLE)

                        # Pad events until all threads reach single-end
                        chunks = [(loc, list(popleftwhile(filter_single_end, q)))
                                  for loc, q in self.trace.events.items()]

                        # Pad events with chunks
                        padded = list(zip(*list(itertools.zip_longest(*list(evts for loc, evts in chunks)))))

                        # Append padded chunks to each location's queue of events
                        for (key, _), chunk_padded in zip(chunks, padded):
                            self.events_padded[key].extend(chunk_padded)

                    elif region_role not in _supported_region_roles:
                        raise TypeError(f"Unexpected region role encountered: {region_role}")

                    else:
                        continue

                # Handle region-end
                elif next_type not in _supported_events:
                    raise TypeError(f"Unexpected event type encountered: {next_type}")

            else:
                print("*** MULTIPLE EVENT TYPES ***")
                for event in next_events.values():
                    print("{}: {} {}".format(
                        type(event),
                        event.region.region_role in _supported_region_roles,
                        self.event_attribute(event, 'event_type')))
                raise TypeError(f"Multiple events: {next_types}")

        return

    def make_graph(self):
        task_nodes = defaultdict(set)
        for k, step in enumerate(list(zip(*list([event_queue for event_queue in self.events_padded.values()])))):
            graph_event, = {self.event_attribute(e, 'event_type') for e in step if e is not None}
            print(graph_event)
