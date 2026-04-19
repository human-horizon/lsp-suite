# json-lsp

JSON Language Server (LSP) implementation in C++.

## Stack

- **Language**: C++
- **Compiler**: zig c++
- **JSON parsing**: simdjson (singleheader, compiled from source)
- **Protocol**: JSON-RPC over stdio
- **Build**: Makefile

## Building

```bash
make        # compile
make clean  # clean
make install # install to /usr/local/bin
```

## Architecture

```
src/
  main.cpp        # stdio loop, reads JSON-RPC requests
  lsp.cpp/h      # LSP handlers
  json-parser.cpp/h # JSON parsing with simdjson
  simdjson.cpp/h  # JSON parser (singleheader)
```

## LSP Methods

- `initialize` — returns server capabilities with hierarchicalDocumentSymbolSupport
- `textDocument/documentSymbol` — returns JSON structure as hierarchical symbols
- `textDocument/hover` — placeholder

## Features

### Outline (hierarchical symbols)
- Shows top-level keys with type info (e.g., `users: array[10]`, `config: object{3}`)
- Supports expansion to show nested structure
- Uses DOM API for fast parsing

### Editor Types (TODO)
- Will parse only visible viewport
- Show types inline in editor

## Dependencies

All dependencies are vendored and compiled from source. No external dependencies.

## Notes

- simdjson provides SIMD-accelerated JSON parsing
- Binary size ~1.2MB (includes simdjson)
- Supports files > 100MB with fast parsing
