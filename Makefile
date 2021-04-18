CUSTOM_OMP_POSTFIX=.co

# Decide whether to link to a custom OMP runtime when linking OMP executable
ifeq ($(OMP_LIB), )
else
  $(info using custom OMP lib at $(OMP_LIB))
  # Link to a custom OMP runtime
  EXE_POSTFIX=$(CUSTOM_OMP_POSTFIX)
  CPP_OMP_FLAGS=-I$(OMP_LIB)/include
  LD_OMP_FLAGS=-L$(OMP_LIB)/lib/ -Wl,-rpath=$(OMP_LIB)/lib/
endif

CC=clang
CFLAGS=-Wall -Werror -Iinclude/ -Wno-unused-function
LDFLAGS=
OMPTLIB=lib/libompt-core.so
EXE=omp-demo$(EXE_POSTFIX)
BINS=$(OMPTLIB) $(EXE)
DEBUG=-g -DDEBUG_LEVEL=3

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
lib/lib%.so: obj/ompt-core-fpic.o obj/ompt-core-callbacks-fpic.o
	@echo LINKING: $@
	@$(CC) $(LDFLAGS) -shared $^ -o $@

# Compile into .o
obj/%-fpic.o: src/%.c include/%.h
	@echo COMPILING: $@
	@$(CC) $(CFLAGS) $(DEBUG) -fPIC -c $< -o $@

run: $(BINS)
	OMP_TOOL_LIBRARIES=`pwd`/$(OMPTLIB) ./$(EXE)

clean:
	@-rm -f lib/* obj/* $(BINS) *$(CUSTOM_OMP_POSTFIX)
