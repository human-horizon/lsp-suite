CC = zig cc
CFLAGS = -Wall -Wextra -O2
AR = ar
ARFLAGS = rcs

SOURCES = lsp_core.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = lsp_core.h
STATIC_LIB = liblsp-core.a

.PHONY: all clean install

all: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -I. -c $< -o $@

clean:
	rm -f *.o $(STATIC_LIB)

install: $(STATIC_LIB)
	@echo "Installing lsp-core..."
	@mkdir -p /Users/a/Space/Projects/HumanHorizon/lsp-suite/lib
	cp $(STATIC_LIB) /Users/a/Space/Projects/HumanHorizon/lsp-suite/lib/
	cp $(HEADERS) /Users/a/Space/Projects/HumanHorizon/lsp-suite/include/
	@echo "Done. Link with: -L/Users/a/Space/Projects/HumanHorizon/lsp-suite/lib -llsp-core"
