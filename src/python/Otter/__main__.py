import argparse
import Otter


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description='Convert an Otter OTF2 trace archive to its execution graph representation',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('anchorfile', help='OTF2 anchor file')
    args = parser.parse_args()

    print(f"Loading OTF2 anchor file: {args.anchorfile}")
    G = Otter.ExecutionGraph(args.anchorfile)
    G.pad_events()
    for location, padded_events in G.events_padded.items():
        print(f"{location.name} has {len(padded_events)} padded events")

    for k, step in enumerate(list(zip(*list([event_queue for event_queue in G.events_padded.values()])))):
        # print("; ".join([str(dir(e.region)) for e in step if hasattr(e, 'region')]))
        print(k, {G.event_attribute(e, 'event_type') for e in step if e is not None})
