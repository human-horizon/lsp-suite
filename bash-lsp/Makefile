CC = zig cc
CFLAGS = -Wall -Wextra -O2 -g -Isrc -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -ltree-sitter

SRC = src/main.c src/parser.c src/scanner.c

OUT = bash-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)