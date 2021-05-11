import otf2
import itertools
from collections import deque, defaultdict


def load_trace(path="/home/adam/git/otter/default-archive-path/default-archive-name.otf2"):
    with otf2.reader.open(path) as t:
        all_events = list(t.events)
        defs = t.definitions
    event_dict = defaultdict(deque)
    for location, event in all_events:
        event_dict[location].append(event)
    return event_dict, defs


if __name__ == "__main__":
    event_dict, definitions = load_trace()
    locations = list(event_dict.keys())
    events = [event_dict[location] for location in locations]
    for k, location in enumerate(locations):
        print(f"LOCATION: Name: {location.name}, Type: {location.type}, Group : {location.group.name}")
        for e in events[k]:
            try:
                print(f"  Type: {type(e)}, Region: {e.region.name if hasattr(e, 'region') else ''} Time:{e.time}, Attributes:{e.attributes or 'no attributes'}")
            except Exception as ex:
                print(ex)

    for step in zip(*events):
        event_types = {type(evt) for evt in step}
        num_event_types = len(event_types)
        if num_event_types == 1:
            event_type, = event_types
            if event_type == otf2.events.ThreadBegin:
                print("All threads started")
            elif event_type == otf2.events.Enter:
                print("All locations entered a region")
            elif event_type == otf2.events.Leave:
                print("All locations left a region")
            else:
                print(f"Unknown event type: {event_type}")
        elif num_event_types > 1:
            print(f"Event types are: {event_types}")