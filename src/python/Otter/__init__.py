import os
import itertools
from typing import Iterable
from collections import deque, defaultdict


try:
    import otf2
    import _otf2
    from otf2 import events, RegionRole
except ModuleNotFoundError as Err:
    print("Failed to import OTF2 module")
    print(Err)
else:
    print("OTF2 package imported frmom {}".format(os.path.abspath(otf2.__file__)))


_supported_events = [events.ThreadBegin, events.ThreadEnd, events.Enter, events.Leave]
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

    class Trace:

        def __init__(self, events: Iterable, definitions: Iterable):
            self.events = defaultdict(deque)
            for k, (location, event) in enumerate(events):
                self.events[location].append(event)
            self.definitions = definitions
            self.attr_lookup = {a.name: a for a in self.definitions.attributes}
            self.num_events    = k+1
            self.num_locations = len(self.definitions.locations)
            self.num_regions   = len(self.definitions.regions)
            self.num_strings   = len(self.definitions.strings)

    def __init__(self, trace_path: str):
        with otf2.reader.open(trace_path) as tr:
            self.trace = self.Trace(tr.events, tr.definitions)
        msg = f"Trace: {trace_path}\n"\
            + f"{'Locations':12}: {self.trace.num_locations}\n"\
            + f"{'Events':12}: {self.trace.num_events}\n"\
            + f"{'Regions':12}: {self.trace.num_regions}\n"\
            + f"{'Strings':12}: {self.trace.num_strings}"
        print(msg)

    def pad_events(self):

        self.events_padded = defaultdict(deque)

        # Expect all locations to have the same type of initial event (ThreadBegin)
        first_type , = {type(event[0]) for event in self.trace.events.values()}
        if not first_type is otf2.events.ThreadBegin:
            raise TypeError(f"Invalid initial event type: {first_type}")

        # While any location still has events to consume
        while max(map(len, self.trace.events.values())) != 0:

            # type of next event for each location
            next_events = {location: event.popleft() for location, event in self.trace.events.items()}
            next_types  = {type(e) for e in next_events.values()}

            if len(next_types) == 1:

                # Get the event type encountered
                next_type = next_types.pop()

                # Raise error if unsupported event type encountered
                if next_type not in _supported_events:
                    raise TypeError(f"Unsupported event type: {next_type}")

                # Handle thread-begin
                if next_type == otf2.events.ThreadBegin:
                    print("thread-begin")
                    for loc, event in next_events.items():
                        self.events_padded[loc].append(event)

                # Handle thread-end
                elif next_type == otf2.events.ThreadEnd:
                    print("THREAD END")
                    for loc, event in next_events.items():
                        self.events_padded[loc].append(event)

                # Handle region-begin
                elif next_type == otf2.events.Enter:

                    # Detect the region type(s) entered
                    region_roles = {e.region.region_role for e in next_events.values()}

                    if not all([role in _supported_region_roles for role in region_roles]):
                        raise TypeError(f"Unsupported region role in: {region_roles}")

                    if len(region_roles) != 1:
                        raise TypeError(f"Multiple region roles encountered for region-enter event: {region_roles}")

                    # We have region-begin events for all threads, all of same region type
                    region_role = region_roles.pop()
                    # print(region_role)

                    if region_role == RegionRole.PARALLEL:
                        print("parallel-begin")
                        for loc, event in next_events.items():
                            self.events_padded[loc].append(event)

                    elif region_role == RegionRole.SECTIONS:
                        print("sections-begin")
                        for loc, event in next_events.items():
                            self.events_padded[loc].append(event)

                    elif region_role == RegionRole.SINGLE:
                        print("single-begin")

                        # for e in next_events.values():
                        #     print(e.attributes)

                        # Exactly one of the threads should have event.attributes['workshare_type'] == 'single_executor'
                        executor , = [loc for loc, event in next_events.items() if
                                      event.attributes[self.trace.attr_lookup['workshare_type']] == 'single_executor']
                        print(f"  executor: {executor.name} ({executor.type})")

                        # Pad events until all threads reach single-end

                        # Lambda that returns true for required single-end event:
                        filter_single_end = lambda e: not (type(e) == otf2.events.Leave \
                                                      and e.region.region_role == RegionRole.SINGLE)

                        chunks = [(loc, list(popleftwhile(filter_single_end, q)))
                                  for loc, q in self.trace.events.items()]

                        for loc, events in chunks:
                            print(f"{loc.name}: {len(events)}")

                        # Pad events with chunks
                        padded = list(zip(*list(itertools.zip_longest(*list(evts for loc, evts in chunks)))))

                        # Append padded chunks to each location's queue of events
                        for (key, _), chunk_padded in zip(chunks, padded):
                            self.events_padded[key].extend(chunk_padded)


                    elif region_role == RegionRole.LOOP:
                        print("[task]loop-begin")
                        for loc, event in next_events.items():
                            self.events_padded[loc].append(event)

                    elif region_role == RegionRole.LOOP:
                        print("workshare-begin")
                        for loc, event in next_events.items():
                            self.events_padded[loc].append(event)

                    elif region_role in [RegionRole.BARRIER, RegionRole.IMPLICIT_BARRIER]:
                        print("barrier-begin")
                        for loc, event in next_events.items():
                            self.events_padded[loc].append(event)

                    elif region_role == RegionRole.TASK_WAIT:
                        print("taskwait-begin")
                        for loc, event in next_events.items():
                            self.events_padded[loc].append(event)

                    else:
                        raise TypeError(f"Unexpected region role encountered: {region_role}")

                    # print(region_roles)
                    # for e in next_events:
                    #     print(e.attributes)

                # Handle region-end
                elif next_type == otf2.events.Leave:
                    print("LEAVE")
                    for loc, event in next_events.items():
                        print(event.attributes)

                else:
                    raise TypeError(f"Unexpected event type encountered: {next_type}")
            else:
                print(f"Multiple events: {next_types}")
            # print(*[e.attributes for e in next_events])

            print("*** PADDED EVENTS ***")
            for key, value in self.events_padded.items():
                print(key)
                for index, val in enumerate(value):
                    print(index, val)

        return


def main():
    G = ExecutionGraph("/home/adam/git/otter/default-archive-path/default-archive-name.otf2")
    return G


if __name__ == "__main__":
    G = main()
    G.pad_events()
    for location, padded_events in G.events_padded.items():
        print(f"{location.name} has {len(padded_events)} padded events")
