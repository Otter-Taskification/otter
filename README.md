# OMPT Project Template

### A template for developing OMP tools with OMP 5.0+

### Outputs

- Standalone OMP app
- Shared library that can be invoked by the runtime with OMP_TOOL_LIBRARIES

### Compiling

- Use OMP_LIB=<path> to compile an OMP app against a specific OMP runtime installed with prefix <path> e.g. /usr/local. Apps compiled with this option will have a .co ("custom OMP") suffix. Omit to use default runtime.

### Usage

- `OMP_LIB=<path> make` to link against a specific OMP runtime installed under the `<path>` prefix
- `OMP_LIB=<path> make run` to run the app linked against a specific OMP runtime and invoke the tool
