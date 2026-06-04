CC = zig cc
CFLAGS = -I/opt/homebrew/include -I../include -Wall -Wextra -O2
LDFLAGS = -L../lib -llsp-core  -L/tmp/tree-sitter/lib -ltree-sitter

SRC = src/main.c src/parser.c src/scanner.c
OUT = dist/rust-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -rf dist