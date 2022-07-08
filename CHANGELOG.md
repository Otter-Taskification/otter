# Changelog

## Unpublished

### Added
- Define the `task_create_ra` event attribute for *task-create* events to capture the address of code which creates a task.
- `otter-ompt` records the *task-create* address as the `codeptr_ra` argument to the *task-create* callback.
- `otter-serial` records the *task-create* address as the return pointer of the stack frame created by a call to `otterTaskBegin`.
- Otter now copies the contents of `/proc/self/maps` to `aux/maps` in the trace output directory to allow later resolution of addresses into source locations.

### Changed
- Improve debug messages in `otter-serial`.
- Enforce mandatory events in `otter-serial` when tracing requested to stop tracing (future updates will make mandatory events optional).

### Deprecated
- Nothing

### Removed
- Nothing

### Fixed
- Nothing

## v0.2.0 [2022-06-28]

### Added

- Merged [#3](https://github.com/Otter-Taskification/otter/pull/3) from @LonelyCat124 which adds Fortran wrapper for `otter-serial`
- Added to `otter-serial`:
  - Add the `OTTER_SRC_ARGS` macro function to insert `__FILE__`, `__func__` and `__line__` args.
  - Add the `otterPhase[Begin|End|Switch]` entrypoints which may be used to indicate named global algorithmic phases. Phases need not be nested within an `otterThreads[Begin|End]` block.

### Changed

- Replace macros in `otter-serial.h` with functions.
- Rename `otter-serial` API entrypoints:
  - `otterParallel[Begin|End]` is now `otterThreads[Begin|End]`
  - `otterSynchroniseChildTasks` is now `otterSynchroniseTasks`
- `otterSynchroniseTasks` now requires an argument of type `otter_task_sync_t` to indicate whether to synchronise descendant tasks or only child tasks.
- These API entrypoints now require file name, function name & line no. as arguments from user code:
  - `otterTraceInitialise`
  - `otterThreadsBegin`
  - `otterTaskBegin`
- Apply API updates to Fortran bindings.

### Removed

- Removed from `otter-serial`:
    - Remove source location arguments from several entrypoints.
    - Remove the `otterTaskSingle[Begin|End]` entrypoints.
  
## v0.1.0 [2022-05-17]

### Added
- `otter-serial` event source allows developers to trace execution of a serial target application.
- `otter-serial` records the source location of various constructs in the target application e.g. task creation points.
- `otter-ompt` event source is now optional - if cmake is invoked with a C compiler that doesn't support OMPT, `otter-ompt` is not built.
- `pyotter` produces a basic HTML report of a trace, which includes a visualisation of the overall program structure and some basic attributes of its tasks. See `python3 -m otter --help` for usage.

## v0.1 [2021-08-25]

### Added
- Initial prototype.
