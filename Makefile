CC=clang
CFLAGS=-Iinclude/
LDFLAGS=-Llib/ -Wl,-rpath=`pwd`/lib/
DEBUG=-g -DDEBUG_LEVEL=3
LIB=lib/libqueue.so
EXE=demo-queue

.PHONY: clean run

all: $(EXE) $(LIB)

lib/libqueue.so: src/dtypes/queue.c include/dtypes/queue.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) $< -shared -fPIC -o $@

demo-queue: src/demo-queue.c include/dtypes/queue.h lib/libqueue.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEBUG) -lqueue $(DEBUG) $< -o $@

run:
	./$(EXE)

clean:
	@rm -rf $(LIB) $(EXE)
	