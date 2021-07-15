$(info CC=$(shell which $(CC)))
$(info CXX=$(shell which $(CXX)))

# Global options
#C         = clang <- pass in as environment variable instead
CC_VERSION = $(shell $(CC)  --version | head -n 1)
INCLUDE    = -Iinclude -I/opt/otf2/include -I/ddn/data/$(USER)/local/include
NOWARN     = -Wno-unused-function -Wno-unused-variable 
CFLAGS     = -Wall -Werror $(NOWARN) $(INCLUDE) -DCC_VERSION="$(CC_VERSION)"
LDFLAGS    = -L/opt/otf2/lib -L/ddn/data/$(USER)/local/lib
DEBUG      = -g

# MAIN OUTPUT
OTTER    = lib/libotter.so

# header files
COMMON_H   = $(wildcard include/*.h)
OTTERHEAD  = $(wildcard include/otter-core/*.h)         $(COMMON_H)
TRACEHEAD  = $(wildcard include/otter-trace/*.h)        $(COMMON_H)
DTYPEHEAD  = $(wildcard include/otter-datatypes/*.h)    $(COMMON_H)

# source files
OTTERSRC   = $(wildcard src/otter-core/*.c)
TRACESRC   = $(wildcard src/otter-trace/*.c)
DTYPESRC   = $(wildcard src/otter-datatypes/*.c)
OMPSRC     = $(wildcard src/otter-demo/*c)
OMPSRC_CPP = $(wildcard src/otter-demo/*.cpp)

# executables
OMPEXE     = $(patsubst src/otter-demo/omp-%.c, omp-%, $(OMPSRC))
OMPEXE_CPP = $(patsubst src/otter-demo/omp-%.cpp, omp-%, $(OMPSRC_CPP))

BINS = $(OTTER) $(OMPEXE) $(OMPEXE_CPP)

.PHONY: clean cleanfiles run

otter:     $(OTTER)
all:       $(BINS)
exe:       $(OMPEXE) $(OMPEXE_CPP)

# link Otter as a dynamic first-party tool to be loaded by the runtime
OTTEROBJ   = $(patsubst src/otter-core/otter-%.c,   obj/otter-%.o, $(OTTERSRC))
TRACEOBJ   = $(patsubst src/otter-trace/trace-%.c,  obj/trace-%.o, $(TRACESRC))
DTYPEOBJ   = $(patsubst src/otter-datatypes/dt-%.c, obj/dt-%.o,    $(DTYPESRC))
$(OTTER): $(OTTEROBJ) $(TRACEOBJ) $(DTYPEOBJ)
	@printf "==> linking %s\n" $@
	$(CC) $(LDFLAGS) -lpthread -lotf2 -shared $^ -o $@

# otter obj files
obj/otter-%.o: src/otter-core/otter-%.c
	@printf "==> compiling %s\n" $@
	$(CC) $(CFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_OTTER) $^ -fPIC -c -o $@

# trace obj files
obj/trace-%.o: src/otter-trace/trace-%.c
	@printf "==> compiling %s\n" $@
	$(CC) $(CFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_TRACE) $^ -fPIC -c -o $@

# dtype obj files
obj/dt-%.o: src/otter-datatypes/dt-%.c
	@printf "==> compiling %s\n" $@
	$(CC) $(CFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_DATATYPES) $^ -fPIC -c -o $@

# standalone OMP apps
$(OMPEXE): $(OMPSRC)
	@printf "==> compiling %s\n" $@
	$(CC) $(CFLAGS) $(DEBUG) -fopenmp src/otter-demo/$@.c -o $@
	@echo $@ links to `ldd $@ | grep "[lib|libi|libg]omp"`

$(OMPEXE_CPP): $(OMPSRC_CPP)
	@printf "==> compiling %s\n" $@
	$(CXX) $(CFLAGS) $(DEBUG) -fopenmp src/otter-demo/$@.cpp -o $@
	@echo $@ links to `ldd $@ | grep "[lib|libi|libg]omp"`

run: $(BINS) cleanfiles
	OMP_TOOL_LIBRARIES=`pwd`/$(OTTER) ./$(EXE)

notes: docs/notes.tex
	cd docs && pdflatex notes.tex && pandoc -s notes.tex -o README.md

clean:
	-rm -f lib/* obj/* $(BINS)

cleanfiles:
	@-rm -rf *.gv* *.svg *.pdf *.png *.txt *.csv* *.log *.json* \
	docs/*.pdf docs/*.aux docs/*.gz docs/*.log
