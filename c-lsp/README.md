# TCC Minimal - C Parser for LSP

## Overview

This is a stripped-down version of TCC (Tiny C Compiler) for use as a C language server.
Removed: code generation, linking, runtime.

## Kept

- Preprocessor (tccpp.c)
- C Parser (tccgen.c)
- Symbol table management
- Type resolution

## Build

```bash
zig cc -o c-lsp main.c tcc.c tccpp.c tccgen.c libtcc.c
```

## Files

```
vendor/tcc/
  tcc.h         - main header with Sym, CType structs
  tccpp.c       - preprocessor
  tccgen.c      - C parser, generates intermediate representation
  tccpp.h       - preprocessor definitions
  tccdefs_.h    - token definitions
  il-opcodes.h   - opcodes
  config.h      - configuration
```

## Removed

- *_gen.c (code generation for x86, arm, etc)
- *_link.c (linking)
- tccrun.c (runtime)
- tccelf.c (ELF handling)
