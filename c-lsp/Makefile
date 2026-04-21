CC = zig cc
CFLAGS = -Wall -Wextra -O2 -I./vendor/tcc
LDFLAGS = ./vendor/tcc/libtcc.a -lm -ldl

SRC = src/main.c
OUT = dist/c-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(OUT)

clean:
	rm -rf dist