# Changelog

<!-- ## Unpublished

### Added

### Changed

### Deprecated

### Removed

### Fixed -->

## v0.1.0 [2022-05-17]

### Added
- `otter-serial` event source allows developers to trace execution of a serial target application.
- `otter-serial` records the source location of various constructs in the target application e.g. task creation points.
- `otter-ompt` event source is now optional - if cmake is invoked with a C compiler that doesn't support OMPT, `otter-ompt` is not built.
- `pyotter` produces a basic HTML report of a trace, which includes a visualisation of the overall program structure and some basic attributes of its tasks. See `python3 -m otter --help` for usage.

## v0.1 [2021-08-25]

### Added
- Initial prototype.
