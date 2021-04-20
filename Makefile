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
NOWARN     = -Wno-unused-function -Wno-unused-variable 
CFLAGS     = -Wall -Werror $(NOWARN) $(INCLUDE)
LDFLAGS    = -Llib/ -Wl,-rpath=`pwd`/lib/
DEBUG      = -g -DDEBUG_LEVEL=3 -DDA_LEN=5 -DDA_INC=5

# MAIN OUTPUT
OTTER    = lib/libotter.so

# SUPPORTING COMPONENTS
OTTLIB     = lib/libotter-ttree.so
ODTLIB     = lib/libotter-dt.so

# Linker commands
L_ODTLIB  = $(patsubst lib/lib%.so, -l%,  $(ODTLIB))
L_ODTDEP  = # none
L_OTTLIB  = $(patsubst lib/lib%.so, -l%,  $(OTTLIB))
L_OTTDEP  = -lgvc -lcgraph -lcdt -lpthread 
L_OTTDEP := $(L_OTTDEP) $(L_ODTLIB)

# Source & header paths
OTTERSRC   = $(wildcard src/otter-core/*.c)
OTTERHEAD  = $(wildcard include/otter-core/*.h)
OTTSRC     = $(wildcard src/otter-task-tree/*.c)
OTTHEAD    = $(wildcard include/otter-task-tree/*.h)
ODTSRC     = $(wildcard src/otter-dtypes/*.c)
ODTHEAD    = $(wildcard include/otter-dtypes/*.h)
OMPSRC     = src/otter-demo/omp-demo.c
OMPEXE     = $(patsubst src/otter-demo/%.c,%,$(OMPSRC))$(EXE_POSTFIX)

BINS = $(OTTER) $(OTTLIB) $(ODTLIB) $(OMPEXE)

.PHONY: all clean run

all: $(BINS)

otter:     $(OTTER)

odt:       $(ODTLIB) 

ott:       $(OTTLIB)

demo:      $(OMPEXE)

### Standalone OMP app
$(OMPEXE): $(OMPSRC)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(DEBUG) $(CPP_OMP_FLAGS) $(LD_OMP_FLAGS) -fopenmp $(OMPSRC) -o $@
	@echo
	@echo $@ links to OMP at: `ldd $@ | grep "[lib|libi|libg]omp"`
	@echo

### OTTer as a dynamic tool to be loaded by the runtime
$(OTTER): $(OTTERSRC) $(OTTERHEAD) $(OTTLIB)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(L_OTTLIB) $(DEBUG) $(OTTERSRC) -shared -fPIC -o $@

### Task-tree lib
$(OTTLIB): $(OTTSRC) $(OTTHEAD) $(ODTLIB)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(L_OTTDEP) $(DEBUG) $(OTTSRC) -shared -fPIC -o $@

### Data-types lib
$(ODTLIB): $(ODTSRC) $(ODTHEAD)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $(ODTSRC) -shared -fPIC -o $@

run: $(BINS)
	OMP_TOOL_LIBRARIES=`pwd`/$(OTTER) ./$(OMPEXE)

clean:
	@-rm -f lib/* obj/* $(BINS) $(OMPEXE)

cleanfiles:
	@-rm -f *.dot *.svg *.pdf *.png

