$(info Compiler: $(CC) [$(shell which $(CC))])

# Global options
#C         = clang <- pass in as environment variable instead
INCLUDE    = -Iinclude -I/opt/otf2/include -I/ddn/data/$(USER)/local/include
NOWARN     = -Wno-unused-function -Wno-unused-variable 
CFLAGS     = -Wall -Werror $(NOWARN) $(INCLUDE)
LDFLAGS    = -Llib/ -L/opt/otf2/lib -L/ddn/data/$(USER)/local/lib -Wl,-rpath=`pwd -P`/lib/,-rpath=/ddn/data/$(USER)/local/lib
OTTER_DEFS = -DOTTER_DEFAULT_TASK_CHILDREN=$(OTTER_DEFAULT_TASK_CHILDREN)
GRAPH_DEFS = -DOTT_DEFAULT_ROOT_CHILDREN=$(OTT_DEFAULT_ROOT_CHILDREN)
DTYPE_DEFS = -DDA_LEN=$(ODT_DEFAULT_ARRAY_LENGTH) -DDA_INC=$(ODT_DEFAULT_ARRAY_INCREMENT)
TRACE_DEFS =
DEBUG      = -g

# MAIN OUTPUT
OTTER    = lib/libotter.so

# SUPPORTING MODULES
LIBGRAPH   = lib/libotter-task-graph.so
LIBTRACE   = lib/libotter-trace.so
LIBDTYPE   = lib/libotter-datatypes.so

# Linker commands
L_LIBDTYPE = $(patsubst lib/lib%.so, -l%,  $(LIBDTYPE))
L_DEPDTYPE = # none
L_LIBGRAPH = $(patsubst lib/lib%.so, -l%,  $(LIBGRAPH))
L_DEPGRAPH = -lpthread $(L_LIBDTYPE)
L_LIBTRACE = $(patsubst lib/lib%.so, -l%,  $(LIBTRACE))
L_DEPTRACE = -lpthread -lotf2 $(L_LIBDTYPE)

# Source & header paths
COMMON_H   = $(wildcard include/*.h)
OTTERSRC   = $(wildcard src/otter-core/*.c)
OTTERHEAD  = $(wildcard include/otter-core/*.h)         $(COMMON_H)
GRAPHSRC   = $(wildcard src/otter-task-graph/*.c)
GRAPHHEAD  = $(wildcard include/otter-task-graph/*.h)   $(COMMON_H)
TRACESRC   = $(wildcard src/otter-trace/*.c)
TRACEHEAD  = $(wildcard include/otter-trace/*.h)        $(COMMON_H)
DTYPESRC   = $(wildcard src/otter-datatypes/*.c)
DTYPEHEAD  = $(wildcard include/otter-datatypes/*.h)    $(COMMON_H)
OMPSRC     = $(wildcard src/otter-demo/*.c)
OMPEXE     = $(patsubst src/otter-demo/omp-%.c, omp-%, $(OMPSRC))

BINS = $(OTTER) $(LIBGRAPH) $(LIBDTYPE) $(LIBTRACE) $(OMPEXE)

.PHONY: all clean cleanfiles run

all:       $(BINS) cleanfiles
otter:     $(OTTER)
datatypes: $(LIBDTYPE) 
taskgraph: $(LIBGRAPH)
trace:     $(LIBTRACE)
exe:       $(OMPEXE)

### Standalone OMP app
$(OMPEXE): $(OMPSRC)
	@echo COMPILING: $@
	$(CC) $(CFLAGS) $(DEBUG) -fopenmp src/otter-demo/$@.c -o $@
	@echo $@ links to `ldd $@ | grep "[lib|libi|libg]omp"`

### Otter as a dynamic tool to be loaded by the runtime
$(OTTER): $(OTTERSRC) $(OTTERHEAD) $(LIBGRAPH) $(LIBTRACE) $(LIBGRAPH)
	printf "COMPILING %-12s (debug=%s, OTTER_DEFS=%s)" $@ $(DEBUG_OTTER) $(OTTER_DEFS)
	@$(CC) $(CFLAGS) $(OTTER_DEFS) $(LDFLAGS) $(L_LIBGRAPH) $(L_LIBTRACE) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_OTTER) $(OTTERSRC) -shared -fPIC -o $@

### Task-graph lib
$(LIBGRAPH): $(GRAPHSRC) $(GRAPHHEAD) $(LIBDTYPE)
	@echo COMPILING: $@ debug=$(DEBUG_OTT), GRAPH_DEFS=$(GRAPH_DEFS)
	$(CC) $(CFLAGS) $(GRAPH_DEFS) $(LDFLAGS) $(L_DEPGRAPH) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_OTT) $(GRAPHSRC) -shared -fPIC -o $@

### Event tracing lib
$(LIBTRACE): $(TRACESRC) $(TRACEHEAD) $(LIBDTYPE)
	@echo COMPILING: $@ debug=$(DEBUG_TRACE), TRACE_DEFS=$(TRACE_DEFS)
	$(CC) $(CFLAGS) $(TRACE_DEFS) $(LDFLAGS) $(L_DEPTRACE) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_TRACE) $(TRACESRC) -shared -fPIC -o $@

### Datatypes lib
$(LIBDTYPE): $(DTYPESRC) $(DTYPEHEAD)
	@echo COMPILING: $@ debug=$(DEBUG_ODT), DTYPE_DEFS=$(DTYPE_DEFS)
	$(CC) $(CFLAGS) $(DTYPE_DEFS) $(LDFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_ODT) $(DTYPESRC) -shared -fPIC -o $@

run: $(BINS)
	@OMP_TOOL_LIBRARIES=`pwd`/$(OTTER) ./$(EXE)

clean:
	@-rm -f lib/* obj/* $(BINS) $(OMPEXE)

cleanfiles:
	@-rm -rf *.gv* *.svg *.pdf *.png *.txt *.csv* *.json* default-archive-path/
