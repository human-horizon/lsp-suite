CC = zig cc
CFLAGS = -I/tmp/tree-sitter/lib/include -I../include -Wall -Wextra -O2
LDFLAGS = ../src/lsp-core/liblsp-core.a /tmp/tree-sitter/libtree-sitter.a

SRC = src/main.c src/parser.c src/scanner.c
OUT = dist/python-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -rf dist