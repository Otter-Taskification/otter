import os
from typing import Iterable
from collections import deque, defaultdict


try:
    import otf2
    import _otf2
except ModuleNotFoundError as Err:
    print("Failed to import OTF2 module")
    print(Err)
else:
    print("OTF2 package imported frmom {}".format(os.path.abspath(otf2.__file__)))


class ExecutionGraph:

    class Trace:

        def __init__(self, events: Iterable = None, definitions: Iterable = None):
            self.events = events
            self.definitions = definitions

    def __init__(self, trace_path=None):
        if trace_path is not None:
            with otf2.reader.open(trace_path) as tr:
                self.trace = self.Trace(list(tr.events), tr.definitions)
        else:
            self.trace = self.Trace()
