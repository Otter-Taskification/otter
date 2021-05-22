from __future__ import annotations
from typing import Iterable
from enum import Enum, auto
from collections import deque
from dataclasses import dataclass
from Otter.types import regions, maptypes
import otf2


class RegionTypeName(Enum):
    def _generate_next_value_(name, start, count, last_values):
        return name


class RegionType(RegionTypeName):
    parallel = auto()
    workshare = auto()
    sync = auto()
    task = auto()
    initial_task = auto()
    implicit_task = auto()
    explicit_task = auto()
    target_task = auto()
    sections = auto()
    single_executor = auto()
    single_other = auto()
    distribute = auto()
    loop = auto()
    taskloop = auto()
    barrier = auto()
    barrier_implicit = auto()
    barrier_explicit = auto()
    barrier_implementation = auto()
    taskwait = auto()
    taskgroup = auto()


class Region:
    _ids = set()

    def __init__(self, unique_id: int, encountering_task: regions.TaskRegion, region_type: str):
        if unique_id in type(self)._ids:
            raise ValueError("{} id {} already exists".format(type(self).__name__, unique_id))
        else:
            self.unique_id = unique_id
            self.region_type = RegionType[region_type]
            self.encountering_task = encountering_task or regions.TaskRegion.Undefined
            type(self)._ids.add(self.unique_id)

    def __del__(self):
        type(self)._ids.discard(self.unique_id)
        del self

    def __eq__(self, other):
        if type(self) == type(other):
            return self.unique_id == other.unique_id
        else:
            raise NotImplemented

    @property
    def key(self):
        return self.region_type, self.unique_id

    def __hash__(self):
        return hash(self.key)

    def __repr__(self):
        repr = "{} ".format(type(self).__name__) + "({})"
        return repr.format(", ".join(["{}={}".format(k, v) for k, v in vars(self).items()]))


class ParallelRegion(Region):
    _ids = set()

    def __init__(self, unique_id: int, requested_parallelism: int, is_league: bool, encountering_task: regions.TaskRegion = None):
        super().__init__(unique_id, encountering_task, 'parallel')
        self.requested_parallelism = requested_parallelism
        self.is_league = bool(is_league)
        self._implicit_tasks = set()

    def add_implicit_task(self, task: regions.TaskRegion):
        if task.region_type != RegionType.implicit_task:
            raise TypeError(str(task.region_type))
        self._implicit_tasks.add(task)

    def add_implicit_tasks(self, tasks: Iterable[regions.TaskRegion]):
        for task in tasks:
            if task.region_type == RegionType.implicit_task:
                self.add_implicit_task(task)

    @property
    def implicit_tasks(self):
        for t in self._implicit_tasks:
            yield t


class WorkshareRegion(Region):
    _ids = set()
    next_id = 0

    def __init__(self, region_type: str, encountering_task: regions.TaskRegion = None):
        if region_type not in ["sections","single_executor","single_other","distribute","loop","taskloop"]:
            raise TypeError("Invalid workshare region type: '{}'".format(region_type))
        super().__init__(unique_id=regions.WorkshareRegion.next_id, encountering_task=encountering_task, region_type=region_type)
        regions.WorkshareRegion.next_id += 1


class SyncRegion(Region):
    _ids = set()
    next_id = 0

    def __init__(self, region_type: str, encountering_task: regions.TaskRegion = None):
        if region_type not in ["barrier","barrier_implicit","barrier_explicit","barrier_implementation","taskwait","taskgroup"]:
            raise TypeError("Invalid sync region type: '{}'".format(region_type))
        super().__init__(unique_id=regions.SyncRegion.next_id, encountering_task=encountering_task, region_type=region_type)
        regions.SyncRegion.next_id += 1


class TaskRegion(Region):
    _ids = set()
    Undefined = None

    def __new__(cls, *args, **kwargs):
        if args[0] == 2 ** 64 - 1 and cls.Undefined is not None:
            return cls.Undefined
        else:
            return object.__new__(cls)

    def __init__(self, unique_id: int, region_type: str, creation_event: otf2.events._EventMeta, encountering_task: regions.TaskRegion = None,
                 parent_task: regions.TaskRegion = None):
        if self is type(self).Undefined:
            return
        if region_type not in ['initial_task', 'implicit_task', 'explicit_task', 'target_task', 'task']:
            raise TypeError("Invalid task region type: '{}'".format(region_type))
        super().__init__(unique_id=unique_id, encountering_task=encountering_task, region_type=region_type)
        self._parent_task = parent_task or type(self).Undefined
        if self._parent_task is not None:
            self._parent_task.add_child(self)
        self._child_tasks = set()
        if encountering_task is not None:
            self.encountering_task_id = encountering_task.unique_id
        if creation_event is not None:
            self.crt_ts = creation_event.time

    def __repr__(self):
        repr = "{} ".format(type(self).__name__) + "({})"
        if self is type(self).Undefined:
            repr = repr.format("<undefined>" + ", child tasks: {}".format(len(self._child_tasks)))
        else:
            repr = repr.format(", ".join(["{}={}".format(k, v) for k, v in vars(self).items() if k not in ['_parent_task', '_child_tasks', 'encountering_task']]) + ", child tasks: {}".format(len(self._child_tasks)))
        return repr

    @property
    def parent_task(self):
        return self._parent_task

    @parent_task.setter
    def parent_task(self, parent: regions.TaskRegion = None):
        if self is regions.TaskRegion.Undefined:
            self._parent_task = self
        else:
            self._parent_task._child_tasks.discard(self)
            self._parent_task = parent or type(self).Undefined
        parent.add_child(self)

    @property
    def key(self):
        return regions.RegionType.task, self.unique_id

    def add_child(self, child: regions.TaskRegion):
        self._child_tasks.add(child)

    @property
    def child_tasks(self):
        for c in self._child_tasks:
            yield c

    @property
    def num_children(self):
        return len(self._child_tasks)

    @property
    def has_children(self):
        return self.num_children > 0

    @property
    def descendant_tasks(self):
        return self.__iter__()

    def __iter__(self):
        yield self
        for c in self._child_tasks:
            for descendant in c.descendant_tasks:
                yield descendant


TaskRegion.Undefined = TaskRegion(2 ** 64 - 1, region_type='task', creation_event=None)
TaskRegion.Undefined.parent_task = TaskRegion.Undefined
