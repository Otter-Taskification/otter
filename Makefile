CUSTOM_OMP_POSTFIX=.co

# Decide whether to link to a custom OMP runtime when linking OMP executable
ifeq ($(OMP_LIB), )
else
	$(info using custom OMP lib at $(OMP_LIB))
	# Link to a custom OMP runtime
	EXE_POSTFIX     = $(CUSTOM_OMP_POSTFIX)
	CPP_OMP_FLAGS   = -I$(OMP_LIB)/include
	LD_OMP_FLAGS    = -L$(OMP_LIB)/lib/ -Wl,-rpath=$(OMP_LIB)/lib/
endif

CC             = clang
CFLAGS         = -Wall -Werror -Iinclude/ -Wno-unused-function -Wno-unused-variable
LDFLAGS        = -Llib/ -Wl,-rpath=`pwd`/lib/

OMPTLIB        = lib/libompt-core.so
OMPTSRC        = $(patsubst lib/lib%-core.so, src/%*.c,  $(OMPTLIB))
OMPTHEAD       = $(wildcard include/ompt-*.h)

TTLIB          = lib/libtask-tree.so
TTSRC          = $(patsubst lib/lib%.so, src/modules/%.c, $(TTLIB))
TTHEAD         = $(patsubst lib/lib%.so, include/modules/%.h, $(TTLIB))

LIBS           = lib/libqueue.so lib/libdynamic-array.so
EXE            = omp-demo$(EXE_POSTFIX)
BINS           = $(OMPTLIB) $(TTLIB) $(LIBS) $(EXE)
DEBUG          = -g -DDEBUG_LEVEL=3 -DDA_LEN=5 -DDA_INC=5

DTYPESRC       = $(wildcard src/dtypes/*.c)


DTYPEHEAD      = $(wildcard include/dtypes/*.h)

.PHONY: all clean run

all: $(BINS)

### 0. Standalone OMP app

omp-%$(EXE_POSTFIX): src/omp-%.c
	@$(CC) $(CFLAGS) $(DEBUG) $(CPP_OMP_FLAGS) $(LD_OMP_FLAGS) -fopenmp $< -o $@
	@echo COMPILING: $@
	@echo
	@echo $@ links to OMP at: `ldd $@ | grep "libomp"`
	@echo

### 1. OMP tool as a dynamic tool to be loaded by the runtime

# Link into .so
$(OMPTLIB): $(OMPTSRC) $(TTLIB)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) -ltask-tree $(DEBUG) $(OMPTSRC) -shared -fPIC -o $@

$(TTLIB): $(TTSRC) $(TTHEAD) $(LIBS)
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -lpthread -lqueue -ldynamic-array $(TTSRC) -shared -fPIC -o $@

lib/libqueue.so: src/dtypes/queue.c include/dtypes/queue.h
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

lib/libdynamic-array.so: src/dtypes/dynamic-array.c include/dtypes/dynamic-array.h
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

run: $(BINS)
	OMP_TOOL_LIBRARIES=`pwd`/$(OMPTLIB) ./$(EXE)

clean:
	@-rm -f lib/* obj/* $(BINS) *$(CUSTOM_OMP_POSTFIX)
