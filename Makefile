CC=clang
CFLAGS=-Iinclude/
LDFLAGS=-Llib/ -Wl,-rpath=`pwd`/lib/
DEBUG=-g -DDEBUG_LEVEL=3 -DDA_LEN=5 -DDA_INC=5
LIBS=lib/libqueue.so lib/libdynamic-array.so
BINS=demo-queue demo-dynamic-array
EXE=demo-queue

.PHONY: clean run

all: $(BINS) $(LIBS)

lib/libqueue.so: src/dtypes/queue.c include/dtypes/queue.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

lib/libdynamic-array.so: src/dtypes/dynamic-array.c include/dtypes/dynamic-array.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

demo-queue: src/demo-queue.c include/dtypes/queue.h lib/libqueue.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -lqueue $(DEBUG) $< -o $@

demo-dynamic-array: src/demo-dynamic-array.c include/dtypes/dynamic-array.h lib/libdynamic-array.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -ldynamic-array $(DEBUG) $< -o $@

run: $(EXE) $(LIBS)
	./$(EXE)

clean:
	@rm -rf $(LIBS) $(EXE)
