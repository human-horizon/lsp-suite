# c-lsp - C Language Server using TCC

C Language Server using the TCC (Tiny C Compiler) parser.

## Building

```bash
make
```

Requires zig cc installed.

## Status

✅ Works - LSP protocol implemented with initialize, documentSymbol, hover, shutdown handlers.

## Architecture

- TCC (libtcc.c) provides the C parser
- Custom JSON-RPC protocol handler
- Uses TCC's internal symbol table for outline/symbols (TODO)

## Protocol Support

- `initialize` - Returns server capabilities
- `textDocument/documentSymbol` - Returns symbol outline (stub: returns empty)
- `textDocument/hover` - Returns hover info (stub: returns generic C symbol)
- `shutdown` - Clean shutdown

## Binary

- Size: ~52KB
- Built with: `zig cc -O2`
- Linked: libtcc.c (TCC sources)

## Files

- `src/main.c` - LSP protocol handler
- `vendor/tcc/` - TCC sources (libtcc.c, tcc.h)