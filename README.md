# Vaxiom

Vaxiom is a small compiler front end for an A-Level-style pseudocode language.
It tokenizes a source file, parses it with a recursive-descent parser, and prints
the resulting abstract syntax tree (AST).

## Build

You need a C11 compiler and `make`:

```sh
make
```

This creates two programs:

- `vaxiom` tokenizes and parses a source file, then prints its AST.
- `lexer` writes the token stream to a file for inspection.

## Run

Parse the included example:

```sh
./vaxiom input.txt
```

Print both tokens and the AST:

```sh
./vaxiom --tokens input.txt
```

Write tokens to `output.txt`:

```sh
./lexer input.txt output.txt
```

Both programs use `input.txt` by default when no source path is supplied.

## Test

```sh
make test
```

The tests compile with warnings treated as errors, parse the original example,
exercise every supported statement form, check expression precedence, inspect
the lexer output, and confirm that invalid syntax is rejected.

## Language overview

The language supports:

- Scalar and array declarations
- Assignment and indexed array access
- Integer, string, character-type, and Boolean values
- Arithmetic, comparison, and Boolean expressions
- `INPUT` and `OUTPUT`
- `IF`/`ELSE`/`ENDIF`
- `FOR`/`NEXT` and `WHILE`/`ENDWHILE`
- Function declarations and `CALL` expressions
- `//` and `#` line comments

Keywords are case-insensitive. Identifiers may begin with a letter or
underscore. Semicolons and the `THEN`/`DO` keywords are optional, which keeps
the original sample program valid.

This project performs syntax analysis only. It does not yet execute programs,
perform static type checking, or generate machine code.

## Source layout

- `lexer.c`, `lexer.h`: authoritative lexer and shared token definitions
- `MyParser.c`: parser and command-line driver
- `ast.c`, `ast.h`: AST storage, printing, and cleanup
- `myflex.c`: token-dump command-line driver
- `grammer.txt`: the implemented grammar
- `tests/`: end-to-end regression tests

`vaxiom.l`, `lex.yy.c`, the Windows `.exe` files, and the small experimental C
files are retained from the original project for reference. They are not part
of the build.
