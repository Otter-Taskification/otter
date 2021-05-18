from collections import defaultdict, deque
from typing import Iterable


class Node:

    node_id = 0

    def __init__(self, attr: dict, label: str = "", shape: str = "circle", color: str = "red", style: str = "filled"):
        for key, value in attr.items():
            setattr(self, key, value)
        self.label = label
        self.shape = shape
        self.color = color
        self.style = style
        self.id = Node.node_id
        Node.node_id += 1

    def __str__(self):
        return f"[label=\"{self.label}\" shape=\"{self.shape}\" color=\"{self.color}\" style=\"{self.style}\"]"

    def __hash__(self):
        return hash((self.node_id))

    def __eq__(self, other):
        if not isinstance(other, Node):
            raise NotImplemented
        return self.node_id == other.node_id


class ParallelNode(Node):

    def __init__(self, attr: dict, id: int, label: str = ""):
        super(ParallelNode, self).__init__(attr, label=label, shape="parallelogram", color="lightgreen", style="filled")


class WorkshareNode(Node):

    def __init__(self, attr: dict, id: int, label: str = ""):
        super(WorkshareNode, self).__init__(attr, label=label, shape="diamond", color="lightblue", style="filled")


class SyncNode(Node):

    def __init__(self, attr: dict, id: int, label: str = ""):
        super(SyncNode, self).__init__(attr, label=label, shape="hexagon", color="orange", style="filled")


class TaskNode(Node):

    def __init__(self, attr: dict, id: int, label: str = ""):
        super(TaskNode, self).__init__(attr, label=label, shape="square", color="red", style="filled")


class Trace:
    """Import an OTF2 trace produced by Otter"""

    def __init__(self, events: Iterable, definitions: Iterable):

        self.definitions = definitions
        self.attr_lookup = {a.name: a for a in self.definitions.attributes}
        self.events = defaultdict(deque)

        for k, (location, event) in enumerate(events):
            # discard task-enter/leave events - not required for execution graph
            if event.attributes[self.attr_lookup['event_type']] not in ['task_enter', 'task_leave']:
                self.events[location].append(event)

        self.num_events = k + 1
        self.num_locations = len(self.definitions.locations)
        self.num_regions = len(self.definitions.regions)
        self.num_strings = len(self.definitions.strings)


class Graph:
    """Simple representation of a trace's execution graph"""

    def __init__(self):
        self.nodes = defaultdict(set)

    def __contains__(self, item):
        if issubclass(type(item), Node):
            return item in self.nodes.keys()
        elif isinstance(item, tuple):
            if not all(list(map(lambda x: issubclass(type(x), Node), item))):
                return NotImplemented
            return item in self.edges
        else:
            raise NotImplemented

    def add_node(self, n: Node):
        self.nodes[n] = set()

    def add_edge(self, src: Node, dest: Node):
        self.nodes[src].add(dest)
        if dest not in self.nodes:
            self.add_node(dest)
