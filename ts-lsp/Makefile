CC = zig cc
CFLAGS = -I/tmp/tree-sitter/lib/include -I/tmp/tree-sitter-typescript/common -I./src -Wall -Wextra -O2
LDFLAGS = /tmp/tree-sitter/libtree-sitter.a

SRC = src/main.c src/typescript/parser.c src/typescript/scanner.c
OUT = dist/ts-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -rf dist