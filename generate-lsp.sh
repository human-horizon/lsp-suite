#!/bin/bash
# Generate LSP server from tree-sitter grammar

if [ $# -lt 2 ]; then
    echo "Usage: $0 <lang> <tree-sitter-dir> [symbol-types...]"
    echo "Example: $0 python /tmp/tree-sitter-python rule function class import"
    exit 1
fi

LANG=$1
LANG_UPPER=$(echo "$LANG" | tr '[:lower:]' '[:upper:]')
TS_DIR=$2
shift 2
SYMBOLS="${@:-function class}"

# Convert symbols to C code
EXTRACT_CODE=""
for sym in $SYMBOLS; do
    EXTRACT_CODE="${EXTRACT_CODE}
    if (strcmp(name, \"${sym}\") == 0) {"
done

# Trim trailing newlines
EXTRACT_CODE=$(echo "$EXTRACT_CODE" | sed 's/^$//')

# Create LSP server directory
LSP_DIR="/Users/a/Space/Projects/HumanHorizon/lsp-suite/${LANG}-lsp"
mkdir -p "$LSP_DIR/src"

# Copy tree-sitter files
cp "${TS_DIR}/src/parser.c" "${LSP_DIR}/src/"
cp "${TS_DIR}/src/scanner.c" "${LSP_DIR}/src/" 2>/dev/null || true
cp -r "${TS_DIR}/src/tree_sitter" "${LSP_DIR}/src/"

# Copy header if exists
if [ -f "${TS_DIR}/bindings/c/tree_sitter/tree-sitter-${LANG}.h" ]; then
    cp "${TS_DIR}/bindings/c/tree_sitter/tree-sitter-${LANG}.h" "${LSP_DIR}/src/"
elif [ -f "${TS_DIR}/src/tree-sitter-${LANG}.h" ]; then
    cp "${TS_DIR}/src/tree-sitter-${LANG}.h" "${LSP_DIR}/src/"
fi

# Generate main.c from template
sed -e "s/{{LANG}}/${LANG}/g" \
    -e "s/{{LANG_NAME}}/${LANG_UPPER}/g" \
    "/Users/a/Space/Projects/HumanHorizon/lsp-suite/template/main.c.template" > "${LSP_DIR}/src/main.c"

# Create Makefile
cat > "${LSP_DIR}/Makefile" << EOF
CC = zig cc
CFLAGS = -Wall -Wextra -O2 -g -Isrc -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -ltree-sitter

SRC = src/main.c src/parser.c src/scanner.c

OUT = ${LANG}-lsp

.PHONY: all clean

all: \$(OUT)

\$(OUT): \$(SRC)
	\$(CC) \$(CFLAGS) \$(SRC) \$(LDFLAGS) -o \$(OUT)

clean:
	rm -f \$(OUT)
EOF

echo "Generated ${LANG}-lsp in $LSP_DIR"
echo "To build: cd $LSP_DIR && make"