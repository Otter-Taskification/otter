CC=clang
CFLAGS=-Iinclude/
LDFLAGS=-Llib/ -Wl,-rpath=`pwd`/lib/
DEBUG=-g -DDEBUG_LEVEL=3 -DDA_LEN=5 -DDA_INC=5
LIB=lib/libdynamic-array.so
EXE=demo-dynamic-array

.PHONY: clean run

all: $(EXE) $(LIB)

lib/libdynamic-array.so: src/dtypes/dynamic-array.c include/dtypes/dynamic-array.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

demo-dynamic-array: src/demo-dynamic-array.c include/dtypes/dynamic-array.h lib/libdynamic-array.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -ldynamic-array $(DEBUG) $< -o $@

run: $(EXE) $(LIB)
	./$(EXE)

clean:
	@rm -rf $(LIB) $(EXE)
	