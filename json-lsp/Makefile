CC = zig cc
CFLAGS = -Wall -Wextra -O2

SRC = src/main.c
OUT = json-lsp

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)