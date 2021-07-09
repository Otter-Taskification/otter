$(info CC=$(shell which $(CC)))
$(info CXX=$(shell which $(CXX)))

# Global options
#C         = clang <- pass in as environment variable instead
INCLUDE    = -Iinclude -I/opt/otf2/include -I/ddn/data/$(USER)/local/include
NOWARN     = -Wno-unused-function -Wno-unused-variable 
CFLAGS     = -Wall -Werror $(NOWARN) $(INCLUDE)
LDFLAGS    = -Llib/ -L/opt/otf2/lib -L/ddn/data/$(USER)/local/lib -Wl,-rpath=`pwd -P`/lib/,-rpath=/ddn/data/$(USER)/local/lib
OTTER_DEFS =
DTYPE_DEFS = -DDA_LEN=$(OTTER_DEFAULT_ARRAY_LENGTH) -DDA_INC=$(OTTER_DEFAULT_ARRAY_INCREMENT)
TRACE_DEFS =
DEBUG      = -g

# MAIN OUTPUT
OTTER    = lib/libotter.so

# SUPPORTING MODULES
LIBTRACE   = lib/libotter-trace.so
LIBDTYPE   = lib/libotter-datatypes.so

# Linker commands
L_LIBDTYPE = $(patsubst lib/lib%.so, -l%,  $(LIBDTYPE))
L_DEPDTYPE = # none
L_LIBTRACE = $(patsubst lib/lib%.so, -l%,  $(LIBTRACE))
L_DEPTRACE = -lpthread -lotf2 $(L_LIBDTYPE)

# Source & header paths
COMMON_H   = $(wildcard include/*.h)
OTTERSRC   = $(wildcard src/otter-core/*.c)
OTTERHEAD  = $(wildcard include/otter-core/*.h)         $(COMMON_H)
TRACESRC   = $(wildcard src/otter-trace/*.c)
TRACEHEAD  = $(wildcard include/otter-trace/*.h)        $(COMMON_H)
DTYPESRC   = $(wildcard src/otter-datatypes/*.c)
DTYPEHEAD  = $(wildcard include/otter-datatypes/*.h)    $(COMMON_H)
OMPSRC     = $(wildcard src/otter-demo/*c)
OMPEXE     = $(patsubst src/otter-demo/omp-%.c, omp-%, $(OMPSRC))
OMPSRC_CPP = $(wildcard src/otter-demo/*.cpp)
OMPEXE_CPP = $(patsubst src/otter-demo/omp-%.cpp, omp-%, $(OMPSRC_CPP))

BINS = $(OTTER) $(LIBDTYPE) $(LIBTRACE) $(OMPEXE) $(OMPEXE_CPP)

.PHONY: all clean cleanfiles run

all:       $(BINS) cleanfiles
otter:     $(OTTER)
datatypes: $(LIBDTYPE)
trace:     $(LIBTRACE)
exe:       $(OMPEXE)

### Standalone OMP app
$(OMPEXE): $(OMPSRC)
	@echo COMPILING: $@
	$(CC) $(CFLAGS) $(DEBUG) -fopenmp src/otter-demo/$@.c -o $@
	@echo $@ links to `ldd $@ | grep "[lib|libi|libg]omp"`

$(OMPEXE_CPP): $(OMPSRC_CPP)
	@echo COMPILING: $@
	$(CXX) $(CFLAGS) $(DEBUG) -fopenmp src/otter-demo/$@.cpp -o $@
	@echo $@ links to `ldd $@ | grep "[lib|libi|libg]omp"`

### Otter as a dynamic tool to be loaded by the runtime
$(OTTER): $(OTTERSRC) $(OTTERHEAD) $(LIBTRACE)
	@printf "COMPILING %-12s (debug=%s, OTTER_DEFS=%s)\n" $@ $(DEBUG_OTTER) $(OTTER_DEFS)
	@$(CC) $(CFLAGS) $(OTTER_DEFS) $(LDFLAGS) $(L_LIBTRACE) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_OTTER) $(OTTERSRC) -shared -fPIC -o $@

### Event tracing lib
$(LIBTRACE): $(TRACESRC) $(TRACEHEAD) $(LIBDTYPE)
	@echo COMPILING: $@ debug=$(DEBUG_TRACE), TRACE_DEFS=$(TRACE_DEFS)
	$(CC) $(CFLAGS) $(TRACE_DEFS) $(LDFLAGS) $(L_DEPTRACE) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_TRACE) $(TRACESRC) -shared -fPIC -o $@

### Datatypes lib
$(LIBDTYPE): $(DTYPESRC) $(DTYPEHEAD)
	@echo COMPILING: $@ debug=$(DEBUG_DATATYPES), DTYPE_DEFS=$(DTYPE_DEFS)
	$(CC) $(CFLAGS) $(DTYPE_DEFS) $(LDFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_DATATYPES) $(DTYPESRC) -shared -fPIC -o $@

run: $(BINS) cleanfiles
	@OMP_TOOL_LIBRARIES=`pwd`/$(OTTER) ./$(EXE)

notes: docs/notes.tex
	cd docs && pdflatex notes.tex && pandoc -s notes.tex -o README.md

clean:
	@-rm -f lib/* obj/* $(BINS) $(OMPEXE)

cleanfiles:
	@-rm -rf *.gv* *.svg *.pdf *.png *.txt *.csv* *.log *.json* \
	default-archive-path/ \
	docs/*.pdf docs/*.aux docs/*.gz docs/*.log
