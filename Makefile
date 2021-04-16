CC=clang
CFLAGS=-Wall -Werror -Iinclude/
DEBUG=-g -DDEBUG_LEVEL=3
LDFLAGS=
EXE=demo-debug-macros

.PHONY: clean run

all: $(EXE)

$(EXE): src/demo-debug-macros.c include/macros/debug.h
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@

run: $(EXE)
	./$(EXE)

clean:
	@-rm -rf $(EXE)