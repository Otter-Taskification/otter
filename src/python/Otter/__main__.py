import argparse
import Otter
from Otter.types import regions


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description='Convert an Otter OTF2 trace archive to its execution graph representation',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('anchorfile', help='OTF2 anchor file')
    args = parser.parse_args()

    print(f"Loading OTF2 anchor file: {args.anchorfile}")
    G = Otter.ExecutionGraph(args.anchorfile)

    G.gather_events_by_region()
    G.print_events_by_region()
