CC = zig cc
CFLAGS = -I/tmp/tree-sitter/include -I../include -Wall -Wextra -O2
LDFLAGS = -L../lib -llsp-core -L/tmp/tree-sitter -ltree-sitter

SRC = src/main.c src/parser.c src/scanner.c src/wrapper.c
OUT = dist/php-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -rf dist