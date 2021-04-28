CUSTOM_OMP_POSTFIX=.co

# Decide whether to link to a custom OMP runtime when linking OMP executable
ifeq ($(OMP_LIB), )
else
  $(info using custom OMP lib at $(OMP_LIB))
  # Link to a custom OMP runtime
  EXE_POSTFIX   = $(CUSTOM_OMP_POSTFIX)
  CPP_OMP_FLAGS = -I$(OMP_LIB)/include
  LD_OMP_FLAGS  = -L$(OMP_LIB)/lib/ -Wl,-rpath=$(OMP_LIB)/lib/
endif

### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

$(info Compiler: $(CC) [$(shell which $(CC))])

# Global options
#C         = clang <- pass in as environment variable instead
INCLUDE    = -Iinclude -I/usr/include/graphviz -I/opt/otf2/include
NOWARN     = -Wno-unused-function -Wno-unused-variable 
CFLAGS     = -Wall -Werror $(NOWARN) $(INCLUDE)
LDFLAGS    = -Llib/ -L/opt/otf2/lib -Wl,-rpath=`pwd`/lib/
OTTER_DEFS = -DOTTER_DEFAULT_TASK_CHILDREN=$(OTTER_DEFAULT_TASK_CHILDREN)
OTT_DEFS   = -DOTT_DEFAULT_ROOT_CHILDREN=$(OTT_DEFAULT_ROOT_CHILDREN)
ODT_DEFS   = -DDA_LEN=$(ODT_DEFAULT_ARRAY_LENGTH) -DDA_INC=$(ODT_DEFAULT_ARRAY_INCREMENT)
TRACE_DEFS =
DEBUG      = -g

# MAIN OUTPUT
OTTER    = lib/libotter.so

# SUPPORTING COMPONENTS
OTTLIB     = lib/libotter-ttree.so
ODTLIB     = lib/libotter-dt.so
TRACELIB   = lib/libotter-trace.so

# Linker commands
L_ODTLIB  = $(patsubst lib/lib%.so, -l%,  $(ODTLIB))
L_ODTDEP  = # none
L_OTTLIB  = $(patsubst lib/lib%.so, -l%,  $(OTTLIB))
L_OTTDEP  = -lpthread $(L_ODTLIB)
L_TRACELIB = $(patsubst lib/lib%.so, -l%,  $(TRACELIB))
L_TRACEDEP = -lpthread $(L_ODTLIB)

# Source & header paths
OTTERSRC   = $(wildcard src/otter-core/*.c)
OTTERHEAD  = $(wildcard include/otter-core/*.h)
OTTSRC     = $(wildcard src/otter-task-tree/*.c)
OTTHEAD    = $(wildcard include/otter-task-tree/*.h)
TRACESRC   = $(wildcard src/otter-trace/*.c)
TRACEHEAD  = $(wildcard include/otter-trace/*.h)
ODTSRC     = $(wildcard src/otter-dtypes/*.c)
ODTHEAD    = $(wildcard include/otter-dtypes/*.h)
OMPSRC     = $(wildcard src/otter-demo/*.c)
OMPEXE     = $(patsubst src/otter-demo/omp-%.c, omp-%, $(OMPSRC))
DEMO       = omp-demo$(EXE_POSTFIX)

BINS = $(OTTER) $(OTTLIB) $(ODTLIB) $(TRACELIB) $(OMPEXE)

.PHONY: all clean run

all: $(BINS)

otter:     $(OTTER)

verbose:   $(OTTER)

odt:       $(ODTLIB) 

ott:       $(OTTLIB)

demo:      $(DEMO)

### Standalone OMP app
$(OMPEXE): $(OMPSRC)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(DEBUG) $(CPP_OMP_FLAGS) $(LD_OMP_FLAGS) -fopenmp src/otter-demo/$@.c -o $@
	@echo
	@echo $@ links to `ldd $@ | grep "[lib|libi|libg]omp"`
	@echo

### OTTer as a dynamic tool to be loaded by the runtime
$(OTTER): $(OTTERSRC) $(OTTERHEAD) $(OTTLIB)
	@echo COMPILING: $@ debug=$(DEBUG_OTTER), OTTER_DEFS=$(OTTER_DEFS)
	@$(CC) $(CFLAGS) $(OTTER_DEFS) $(LDFLAGS) $(L_OTTLIB) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_OTTER) $(OTTERSRC) -shared -fPIC -o $@

### Task-tree lib
$(OTTLIB): $(OTTSRC) $(OTTHEAD) $(ODTLIB)
	@echo COMPILING: $@ debug=$(DEBUG_OTT), OTT_DEFS=$(OTT_DEFS)
	@$(CC) $(CFLAGS) $(OTT_DEFS) $(LDFLAGS) $(L_OTTDEP) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_OTT) $(OTTSRC) -shared -fPIC -o $@

### Event tracing lib
$(TRACELIB): $(TRACESRC) $(TRACEHEAD) $(ODTLIB)
	@echo COMPILING: $@ debug=$(DEBUG_TRACE), TRACE_DEFS=$(TRACE_DEFS)
	$(CC) $(CFLAGS) $(TRACE_DEFS) $(LDFLAGS) $(L_TRACEDEP) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_TRACE) $(TRACESRC) -shared -fPIC -o $@

### Data-types lib
$(ODTLIB): $(ODTSRC) $(ODTHEAD)
	@echo COMPILING: $@ debug=$(DEBUG_ODT), ODT_DEFS=$(ODT_DEFS)
	@$(CC) $(CFLAGS) $(ODT_DEFS) $(LDFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_ODT) $(ODTSRC) -shared -fPIC -o $@

verbose: $(OTTERSRC) $(OTTERHEAD) $(OTTLIB)
	@$(CC) $(CFLAGS) $(OTTER_DEFS) $(LDFLAGS) $(L_OTTLIB) $(DEBUG) -DDEBUG_LEVEL=3 $(OTTERSRC) -shared -fPIC -o $(OTTER).verbose

run: $(BINS)
	OMP_TOOL_LIBRARIES=`pwd`/$(OTTER) ./$(EXE)

clean:
	@-rm -f lib/* obj/* $(BINS) $(OMPEXE)

cleanfiles:
	@-rm -f *.gv *.svg *.pdf *.png *.txt *.csv
