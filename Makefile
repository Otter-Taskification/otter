CC=clang
CFLAGS=-Iinclude/
LDFLAGS=-Llib/ -Wl,-rpath=`pwd`/lib/
DEBUG=-g -DDEBUG_LEVEL=2 -DDA_LEN=5 -DDA_INC=5
LIBS=lib/libqueue.so lib/libdynamic-array.so lib/libtask-tree.so
BINS=demo-queue demo-dynamic-array demo-task-tree
EXE=demo-task-tree

.PHONY: clean run

all: $(BINS) $(LIBS)

lib/libqueue.so: src/dtypes/queue.c include/dtypes/queue.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

lib/libdynamic-array.so: src/dtypes/dynamic-array.c include/dtypes/dynamic-array.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

lib/libtask-tree.so: src/modules/task-tree.c include/modules/task-tree.h lib/libqueue.so lib/libdynamic-array.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -lqueue -ldynamic-array $< -shared -fPIC -o $@

demo-queue: src/demo-queue.c include/dtypes/queue.h lib/libqueue.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -lqueue $(DEBUG) $< -o $@

demo-dynamic-array: src/demo-dynamic-array.c include/dtypes/dynamic-array.h lib/libdynamic-array.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -ldynamic-array $(DEBUG) $< -o $@

demo-task-tree: src/demo-task-tree.c include/modules/task-tree.h lib/libtask-tree.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -ltask-tree $(DEBUG) $< -o $@

run: $(EXE) $(LIBS)
	./$(EXE)

clean:
	@rm -rf $(LIBS) $(BINS)
