CC = zig cc
CFLAGS = -Wall -Wextra -O2

SRC = src/main.c
OUT = dist/json-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -rf dist