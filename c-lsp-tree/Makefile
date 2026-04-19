CC = zig cc
CFLAGS = -I/opt/homebrew/include -Wall -Wextra
LDFLAGS = -L/opt/homebrew/lib -ltree-sitter

SRC = src
BUILD = build
TARGET = c-lsp-tree

SRC_FILES = $(SRC)/main.c $(SRC)/parser.c

$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean