


class Node:

    node_id = 0

    def __init__(self, label: str = "", shape: str = "circle", color: str = "red"):
        self.id = Node.node_id
        Node.node_id += 1
        self.label = label
        self.shape = shape
        self.color = color

    def __str__(self):
        return f"[label=\"{self.label}\" shape=\"{self.shape}\" color=\"{self.color}\"]"

    def __hash__(self):
        return hash((self.node_id))

    def __eq__(self, other):
        if not isinstance(other, Node):
            raise NotImplemented
        return self.node_id == other.node_id


class ParallelNode(Node):

    def __init__(self, label: str = ""):
        super(ParallelNode, self).__init__(label=label, shape="parallelogram", color="lightgreen")


class WorkshareNode(Node):

    def __init__(self, label: str = ""):
        super(WorkshareNode, self).__init__(label=label, shape="diamond", color="lightblue")


class SyncNode(Node):

    def __init__(self, label: str = ""):
        super(SyncNode, self).__init__(label=label, shape="hexagon", color="blue")
