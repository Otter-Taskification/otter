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
INCLUDE    = -Iinclude/ -I/usr/include/graphviz/
TURNEDOFF  = -Wno-unused-function -Wno-unused-variable 
CFLAGS     = -Wall -Werror $(TURNEDOFF) $(INCLUDE)
LDFLAGS    = -Llib/ -Wl,-rpath=`pwd`/lib/
DEBUG      = -g -DDEBUG_LEVEL=3 -DDA_LEN=5 -DDA_INC=5

# MAIN OUTPUT
OMPTLIB    = lib/libompt-core.so

# SUPPORTING COMPONENTS
TTLIB      = lib/libtask-tree.so
DSLIBS     = lib/libqueue.so lib/libdynamic-array.so
EXE        = omp-demo$(EXE_POSTFIX)

# Linker commands
LD_DSLIBS  = $(patsubst lib/lib%.so, -l%,  $(ODTLIB))
LD_TTLIB   = $(patsubst lib/lib%.so, -l%,  $(TTLIB))
LD_TTDEPS  = -lgvc -lcgraph -lcdt -lpthread $(LD_DSLIBS)

# Source & header paths
OMPTSRC    = $(patsubst lib/lib%-core.so, src/%*.c,  $(OMPTLIB))
OMPTHEAD   = $(patsubst lib/lib%-core.so, include/%*.h,  $(OMPTLIB))
TTSRC      = $(patsubst lib/lib%.so, src/modules/%*.c, $(TTLIB))
TTHEAD     = $(patsubst lib/lib%.so, include/modules/%.h, $(TTLIB))
OMPSRC     = $(wildcard src/omp-*.c)
OMPEXE     = $(patsubst src/omp-%.c, omp-%,  $(OMPSRC))

# Otter Dtypes lib
ODTLIB     = lib/libotter-dt.so
ODTSRC     = $(wildcard src/otter-dt/*.c)
ODTHEAD    = $(wildcard include/otter-dt/*.h)

BINS = $(OMPTLIB) $(TTLIB) $(ODTLIB) $(OMPEXE)

.PHONY: all clean run

all: $(BINS)

otter:     $(OMPTLIB)

odt:       $(ODTLIB) 

tasktree:  $(TTLIB)

### Standalone OMP app
$(OMPEXE)$(EXE_POSTFIX): $(OMPSRC)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(DEBUG) $(CPP_OMP_FLAGS) $(LD_OMP_FLAGS) -fopenmp $< -o $@
	@echo
	@echo $@ links to OMP at: `ldd $@ | grep "[lib|libi|libg]omp"`
	@echo

### OMP tool as a dynamic tool to be loaded by the runtime
$(OMPTLIB): $(OMPTSRC) $(TTLIB)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LD_TTLIB) $(DEBUG) $(OMPTSRC) -shared -fPIC -o $@

### Task-tree lib (support component of OMPTLIB)
$(TTLIB): $(TTSRC) $(TTHEAD) $(ODTLIB)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(LD_TTDEPS) $(DEBUG) $(TTSRC) -shared -fPIC -o $@

$(ODTLIB): $(ODTSRC) $(ODTHEAD)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $(ODTSRC) -shared -fPIC -o $@

lib/lib%.so: src/dtypes/%.c include/dtypes/%.h
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

run: $(BINS)
	OMP_TOOL_LIBRARIES=`pwd`/$(OMPTLIB) ./$(EXE)

clean:
	@-rm -f lib/* obj/* $(BINS) *$(CUSTOM_OMP_POSTFIX)
