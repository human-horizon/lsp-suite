# lsp-suite

Suite of Language Server (LSP) implementations for Vision code visualization.

## Structure

```
lsp-suite/
├── AGENTS.md              # This file
├── json-lsp/             # JSON LSP (C++ + simdjson)
├── c-lsp/                # C LSP (TCC parser)
├── html-lsp/             # HTML LSP (C + tree-sitter)
├── css-lsp/              # CSS LSP (C + tree-sitter)
├── python-lsp/           # Python LSP (C + tree-sitter)
├── rust-lsp/             # Rust LSP (C + tree-sitter)
├── java-lsp/             # Java LSP (C + tree-sitter)
├── go-lsp/               # Go LSP (C + tree-sitter)
├── php-lsp/              # PHP LSP (C + tree-sitter)
├── bash-lsp/             # Bash LSP (C + tree-sitter)
├── ruby-lsp/             # Ruby LSP (C + tree-sitter)
├── scala-lsp/            # Scala LSP (C + tree-sitter)
├── swift-lsp/            # Swift LSP (C + tree-sitter)
├── c-lsp-tree/           # C LSP (C + tree-sitter)
├── cpp-lsp-tree/         # C++ LSP (C + tree-sitter)
├── json-lsp-tree/        # JSON LSP (C + tree-sitter)
├── ts-lsp/               # TypeScript LSP (tree-sitter + oxc)
└── js-lsp/               # JavaScript LSP (tree-sitter + oxc)
```

## Technologies

| LSP | Language | Parser | Status |
|-----|----------|--------|--------|
| json-lsp | C | manual parsing | ✅ Works |
| c-lsp | C | TCC parser | ✅ Works |
| html-lsp | C | tree-sitter | ✅ Works |
| css-lsp | C | tree-sitter | ✅ Works |
| python-lsp | C | tree-sitter | ✅ Works |
| rust-lsp | C | tree-sitter | ✅ Works |
| java-lsp | C | tree-sitter | ✅ Works |
| go-lsp | C | tree-sitter | ✅ Works |
| php-lsp | C | tree-sitter | ✅ Works |
| bash-lsp | C | tree-sitter | ✅ Works |
| ruby-lsp | C | tree-sitter | ✅ Works |
| scala-lsp | C | tree-sitter | ✅ Works |
| swift-lsp | C | tree-sitter | ✅ Works |
| c-lsp-tree | C | tree-sitter | ✅ Works |
| cpp-lsp-tree | C | tree-sitter | ✅ Works |
| json-lsp-tree | C | tree-sitter | ✅ Works |
| ts-lsp | JS/TS | tree-sitter + oxc | ✅ Works |
| js-lsp | JS/TS | tree-sitter + oxc | ✅ Works |

## Building

### Prerequisites

- zig cc/c++ installed
- tree-sitter (for tree-sitter based LSPs): `brew install tree-sitter`
- Rust + cargo (for oxc FFI library)

### Build all tree-sitter based LSPs

```bash
make -C python-lsp
make -C rust-lsp
make -C java-lsp
make -C go-lsp
make -C php-lsp
make -C bash-lsp
make -C ruby-lsp
make -C scala-lsp
make -C swift-lsp
make -C c-lsp-tree
make -C cpp-lsp-tree
make -C json-lsp-tree
```

### Build JS/TS LSPs (tree-sitter + oxc)

```bash
# Build oxc FFI library
cd /Users/a/Space/Projects/Lattice/oxc
cargo build --release -p liboxc-ffi

# Generate tree-sitter parsers (requires npm + tree-sitter CLI)
cd /tmp/tree-sitter-javascript && npm install && tree-sitter generate
cd /tmp/tree-sitter-typescript/typescript && npm install && tree-sitter generate

# Build JS/TS LSPs
cd /Users/a/Space/Projects/HumanHorizon/lsp-suite/js-lsp
make
cd ../ts-lsp
make
```

## JS/TS LSP Architecture

js-lsp and ts-lsp use **both** parsers:

- **tree-sitter** - fast, used by default for outline/highlighting
- **oxc** - available for detailed symbol analysis when needed

### FFI Functions (oxc)

```c
OxcSymbols oxc_parse_js(const char* source);
OxcSymbols oxc_parse_ts(const char* source);
void oxc_free_symbols(OxcSymbols* symbols);
```

## tree-sitter Based LSP Architecture

1. **tree-sitter** grammars → cloned in `/tmp/tree-sitter-*`
2. Generated C parsers via `tree-sitter generate`
3. Links against tree-sitter dylib from Homebrew

## LSP Protocol Handling

- Reads headers until blank line (CRLF handling)
- Content-Length header specifies body size
- JSON-RPC 2.0 messages over stdio

## Notes

- All LSPs communicate via JSON-RPC over stdio
- Uses zig cc/c++ for compilation
- **oxc** - fastest JS/TS parser (Rust), for detailed analysis
- **tree-sitter** - fast enough for most cases, better for AST analysis
- simdjson for JSON parsing (fast, singleheader)