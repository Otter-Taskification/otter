class Node:

    def __init__(self, id: int, label: str = "", shape: str = "circle", color: str = "red"):
        self.id = id
        self.label = label
        self.shape = shape
        self.color = color

    def __str__(self):
        return f"[label=\"{self.label}\" shape=\"{self.shape}\" color=\"{self.color}\"]"


class ParallelNode(Node):

    def __init__(self, id: int, label: str = ""):
        super(ParallelNode, self).__init__(id=id, label=label, shape="parallelogram", color="lightgreen")


class WorkshareNode(Node):

    def __init__(self, id: int, label: str = ""):
        super(WorkshareNode, self).__init__(id=id, label=label, shape="diamond", color="lightblue")


class SyncNode(Node):

    def __init__(self, id: int, label: str = ""):
        super(SyncNode, self).__init__(id=id, label=label, shape="hexagon", color="blue")
