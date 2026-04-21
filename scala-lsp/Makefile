CC = zig cc
CFLAGS = -I/opt/homebrew/include -Wall -Wextra -O2
LDFLAGS = -L/opt/homebrew/lib -ltree-sitter

SRC = src/main.c src/parser.c src/scanner.c
OUT = dist/scala-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -rf dist