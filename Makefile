$(info CC=$(shell which $(CC)))
$(info CXX=$(shell which $(CXX)))

# Global options
#C         = clang <- pass in as environment variable instead
INCLUDE    = -Iinclude -I/opt/otf2/include -I/ddn/data/$(USER)/local/include
NOWARN     = -Wno-unused-function -Wno-unused-variable 
CFLAGS     = -Wall -Werror $(NOWARN) $(INCLUDE)
LDFLAGS    = -L/opt/otf2/lib -L/ddn/data/$(USER)/local/lib
DEBUG      = -g

# MAIN OUTPUT
OTTER    = lib/libotter.so

# Source & header paths
COMMON_H   = $(wildcard include/*.h)
OTTERSRC   = $(wildcard src/otter-core/*.c)
OTTERHEAD  = $(wildcard include/otter-core/*.h)         $(COMMON_H)
OTTEROBJ   = $(patsubst src/otter-core/%.c, obj/%.o,   $(OTTERSRC))
TRACESRC   = $(wildcard src/otter-trace/*.c)
TRACEHEAD  = $(wildcard include/otter-trace/*.h)        $(COMMON_H)
TRACEOBJ   = $(patsubst src/otter-trace/%.c, obj/%.o,   $(TRACESRC))
DTYPESRC   = $(wildcard src/otter-datatypes/*.c)
DTYPEHEAD  = $(wildcard include/otter-datatypes/*.h)    $(COMMON_H)
DTYPEOBJ   = $(patsubst src/otter-datatypes/%.c, obj/%.o,   $(DTYPESRC))
OMPSRC     = $(wildcard src/otter-demo/*c)
OMPEXE     = $(patsubst src/otter-demo/omp-%.c, omp-%, $(OMPSRC))
OMPSRC_CPP = $(wildcard src/otter-demo/*.cpp)
OMPEXE_CPP = $(patsubst src/otter-demo/omp-%.cpp, omp-%, $(OMPSRC_CPP))

BINS = $(OTTER) $(OMPEXE) $(OMPEXE_CPP)

.PHONY: clean cleanfiles run

otter:     $(OTTER)
all:       $(BINS)
exe:       $(OMPEXE) $(OMPEXE_CPP)

# otter obj files
obj/ot%.o: $(patsubst obj/%.o, src/otter-core/%.c, $@)
	@printf "==> compiling %s\n" $@
	$(CC) $(CFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_OTTER) $(patsubst obj/%.o, src/otter-core/%.c, $@) -fPIC -c -o $@

# trace obj files
obj/tr%.o: $(patsubst obj/%.o, src/otter-trace/%.c, $@)
	@printf "==> compiling %s\n" $@
	$(CC) $(CFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_TRACE) $(patsubst obj/%.o, src/otter-trace/%.c, $@) -fPIC -c -o $@

# dtype obj files
obj/%.o: $(patsubst obj/%.o, src/otter-datatypes/%.c, $@)
	@printf "==> compiling %s\n" $@
	$(CC) $(CFLAGS) $(DEBUG) -DDEBUG_LEVEL=$(DEBUG_DATATYPES) $(patsubst obj/%.o, src/otter-datatypes/%.c, $@) -fPIC -c -o $@

# link otter as a dynamic first-party tool to be loaded by the runtime
$(OTTER): $(OTTEROBJ) $(TRACEOBJ) $(DTYPEOBJ)
	@printf "==> linking %s\n" $@
	$(CC) $(LDFLAGS) -lpthread -lotf2 -shared $^ -o $@

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
