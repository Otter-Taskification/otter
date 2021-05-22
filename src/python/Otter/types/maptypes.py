from __future__ import annotations
from . import regions


class TaskIdMap:

    _tasks = dict()

    def __setitem__(self, unique_id: int, task: regions.TaskRegion):
        if unique_id in self._tasks:
            raise KeyError("Task already present")
        else:
            self._tasks[unique_id] = task

    def __getitem__(self, unique_id: int):
        return self._tasks[unique_id]

    def __len__(self):
        return len(self._tasks)

    def __iter__(self):
        return ((k, v) for (k, v) in self._tasks.items())

    def get(self, unique_id: int, default: int):
        return self._tasks.get(unique_id) if unique_id in self._tasks else default
