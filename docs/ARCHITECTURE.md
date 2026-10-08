# Architecture (v0.2 implementation)

NatLang is written in **C++20**. The host C++ compiler is an external build-time dependency; the generated application embeds a small C++ runtime. All current native code is produced through a C++ source generation backend rather than direct assembly or LLVM IR.

## Components

| Component | File | Responsibility |
|---|---|---|
| CLI & pipeline | `src/natc.cpp` | Option parsing, input source, fallback strategy, output and host compilation |
| Canonical phrase parser | `src/natc.cpp` (`Parser`) | Recognize supported English-shaped statements and nested `End` blocks |
| Expression frontend | `src/natc.cpp` (`lex`, `Expr`) | Tokenization, precedence parsing, check variable/function names and arity |
| Structural AST | `src/natc.cpp` (`Node`) | Statements, arguments, child/else blocks and source line references |
| C++ backend | `src/natc.cpp` (`Emit`) | Generate standalone translation unit with main, functions and expressions |
| Runtime | `src/runtime.hpp` | Tagged dynamic values, arithmetic, lists, files, input/output, validated IPv4 ICMP discovery |
| Embedding step | `src/embedded_runtime.in.hpp` + `CMakeLists.txt` | Bake runtime header source into compiler at build time |
| Optional local AI adapter | `src/natc.cpp` (`llm`) | Send a full program to loopback llama-server, parse constrained JSON reply |

## Compilation path

1. Load UTF-8-ish `.nat` text (ASCII keyword parsing; quoted strings may contain UTF-8 bytes).
2. Parse into statement nodes (`Node`) and validate expressions as C++ snippets are generated.
3. If `--llm` was set **and deterministic parsing failed**, send source to a local server to normalize it, then repeat compilation. `--llm-all` normalizes unconditionally.
4. Emit a generated `.cpp` file containing the runtime implementation plus translated program.
5. Unless `--emit-cpp` or `--check`, invoke a host C++ compiler with `-std=c++20` or `/std:c++20`.
6. Delete generated C++ on success unless `--keep-cpp` was set.

## Value model

The runtime `nat::Value` is a variant containing null/uninitialized, 64-bit floating-point number, string, Boolean and list of `Value`. Variables currently use function-wide or program-wide storage; this is **not** lexical scoping. Every arithmetic operation and built-in checks its dynamic operands at runtime. No object model or custom record type exists.

## Semantic limitations to address

- Expressions are validated during code generation; there is no standalone typed expression IR today.
- Variable names are checked for existence within their function/main but not for definite assignment along every control-flow path; uninitialized reads raise runtime errors.
- Variable names are case-sensitive even though statement keywords match case-insensitively.
- The current parser requires one `End` per block, regardless of indentation.
- Function declarations are top-level, user functions have fixed parameter counts and return `Value`.
- Compiler/tool commands currently use `std::system`; invoke this tool on trusted local paths and arguments only. Replace it with a platform-aware child process runner before treating inputs as untrusted.
- The local AI adapter calls the local `curl` CLI and uses a small in-tree JSON-string extraction helper. A real JSON library and richer error handling are appropriate next steps.
- `Scan IP`/`Scan network` use the OS `ping` executable with strictly validated numeric IPv4 addresses. Inference about host availability is limited to ICMP responses. `Scan network` uses a bounded private-LAN-only interface-discovery strategy and is not a full network scanner.
- The emitted program is not sandboxed; code generated from untrusted or model-normalized source must be reviewed.

## Design proposal for v0.2+

A future architecture should introduce typed immutable IR such as:

```text
Program
  Module[]
    Definition[] (Function, Type, Constant)
    Statement[]
      Let(name, expression)
      If(condition, then, else)
      While(condition, body)
      Call(symbol, arguments)
      Return(expression)
```

Semantic phases then become distinct: normalization -> parsing -> name resolution -> type inference/checking -> definite-assignment/dataflow analysis -> IR validation -> optimizations -> code generation. The natural-language frontend should target exactly this IR. It must not directly inject arbitrary code fragments into the backend.

## Testing

The integration suite compiles `.nat` samples into native executables and runs them, asserts outputs, covers invalid input and diagnostics, and mocks the local llama-server endpoint. **Mock success says nothing about real LLM accuracy.**

See [language reference](LANGUAGE_REFERENCE.md) and [LLM integration](LOCAL_LLM.md).
