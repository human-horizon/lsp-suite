CC = zig cc
CFLAGS = -Wall -Wextra -O2 -g -Ivendor/tcc
LDFLAGS = -lm

SRC = src/main.c vendor/tcc/libtcc.c

OUT = c-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)
