# Changelog

## Unpublished

### Added
- CMake 3.12 build system.
- Detect deprecation of `ompt_callback_master` for `ompt_callback_masked` using compiler ID and version.
- CMake option `-DOTTER_DEBUG_LEVELS=X,Y,Z` for specifying debug level of callbacks (X), tracing (Y) and data types (Z) e.g. 3,0,0.
- CMake option `-DOTTER_WITH_EXAMPLES=[ON|OFF]` to generate and build example programs for demonstrating Otter. Off by default.
- CMake options `-DOTF2_INCLUDE_DIR=<...>` and `-DOTF2_LIB_DIR=<...>` for locating OTF2 components. Default to `/opt/otf2/include` and `/opt/otf2/lib`.
- Warn when GNU compiler detected to protect against https://github.com/adamtuft/otter/issues/14 at runtime.
- Add debug assert statements to detect https://github.com/adamtuft/otter/issues/14 at runtime in debug builds.

### Changed
- Nothing of note.

### Deprecated
- Makefile no longer the supported build method, will be deleted in future updates

### Removed
- Nothing of note.

### Fixed
- Fixed https://github.com/adamtuft/otter/issues/15#issue-988922376 to ensure `sync_cluster_id` vertex attribute is always defined to prevent crashes when there are no barrier regions encountered.

## v0.1 [2021-08-25]

### Added
- Initial prototype.
