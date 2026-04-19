# html-lsp - HTML Language Server using tree-sitter

HTML Language Server using tree-sitter HTML parser.

## Building

```bash
make
```

Requires zig cc and tree-sitter library (from Homebrew: `brew install tree-sitter`).

## Status

✅ Works - LSP protocol implemented with initialize, documentSymbol, hover, shutdown handlers.

## Architecture

- **tree-sitter** - generates C parser from grammar
- **libtree-sitter** - runtime library from Homebrew
- Custom JSON-RPC protocol handler

## Protocol Support

- `initialize` - Returns server capabilities
- `textDocument/documentSymbol` - Parses HTML and returns element symbols
- `textDocument/hover` - Returns hover info
- `shutdown` - Clean shutdown

## Binary

- Size: ~72KB
- Built with: `zig cc -O2`
- Linked with: tree-sitter dylib from Homebrew

## Files

- `src/main.c` - LSP protocol handler + tree-sitter integration
- `src/parser.c` - tree-sitter generated parser
- `src/scanner.c` - tree-sitter external scanner
- `src/tree-sitter-html.h` - tree-sitter language definition