# NatLang — Native Natural-Language Compiler

**v0.4.0 | Multilingual frontend · native C++20 output · math · optional local GGUF frontend + benchmark**

NatLang is an experimental programming-language compiler. Write programs using readable sentences in **Italian or English**, mix languages inside a file, and build a real native executable through C++20. Common Spanish, French and German statements are recognized, while an **optional, user-provided local LLM** can try to normalize less structured multilingual instructions.

**Important:** This is a functional research prototype, **not** a fully syntax-free, universal natural-language compiler. No GGUF/model weights are included, and real quantized-model accuracy has not been validated. The deterministic frontend recognizes an explicit, documented subset. AI-generated normalized programs need review.

## Try it

```text
2 + 2
2 plus 2
2 più 2
Mostra 2 elevato a 5
Mostra radice quadrata di 81
Mostra 15 per cento di 200
Chiedi all'utente un numero e salva in x
Se x è maggiore di 10
    Mostra "Grande"
Altrimenti
    Mostra "Piccolo"
Fine
```

The first six expressions print `4`, `4`, `4`, `32`, `9`, `30`. Later statements prompt and branch according to the input.

```text
Imposta n a 3
Repeat 2 times
    Increase n by 1
Fine
Show n
```

This compiles to a native executable and prints `5`. Italian and English statements share the same internal statement IR and backend.

## Build the compiler

Requires **CMake ≥ 3.16** and a **C++20 compiler**. `natc` itself is written in C++20 and needs neither Python nor an LLM to run. A host C++ toolchain is required to compile `.nat` into native executables.

**Windows (Developer PowerShell with Visual Studio Build Tools / MSVC):**

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\natc.exe examples\multilingual_math.nat -o math.exe --compiler cl
.\math.exe
```

**Linux / macOS (Clang or GCC):**

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/natc examples/multilingual_math.nat -o math
./math
```

Single expression:

```sh
./build/natc --eval "2 plus 2"   # 4
./build/natc --eval "2 ^ 8"      # 256
```

Inspect / compile:

```sh
./build/natc examples/bilingual_program.nat --check --explain
./build/natc examples/bilingual_program.nat --emit-ir program.ir.json
./build/natc examples/bilingual_program.nat --emit-cpp program.cpp
./build/natc examples/bilingual_program.nat -o program --keep-cpp
```

## Model-assisted free-form language (v0.4)

NatLang now includes a small-model prompt profile (`qwen3`), schema-constrained structured responses, one bounded validation-repair retry and a **preview mode that never executes generated code**. You need a separately installed local `llama-server` and a GGUF of your choice.

```powershell
# Windows: first terminal, after winget install llama.cpp
.\scripts\start_gguf.ps1 -Model qwen3
# second terminal (after building natc)
.\build\Release\natc.exe examples\gguf_free_form_italiano.nat --llm-preview
python benchmarks\evaluate.py --natc .\build\Release\natc.exe --mode gguf --model-label Qwen3-0.6B-Q4_K_M --report qwen3-results.json
```

`--llm` only calls the model when deterministic parsing fails; `--llm-all` always normalizes; `--llm-preview` always normalizes and shows canonical source plus IR (no execution). Review the results: semantic interpretation is **not** guaranteed. [GGUF setup, benchmark and limitations](docs/GGUF_EVALUATION.md).

## Supported language constructs

| Capability | Italian | English |
| --- | --- | --- |
| Assignment | `Imposta x a 5` | `Set x to 5` |
| Printing | `Mostra x * 2` | `Show x * 2` |
| Input | `Chiedi all'utente un numero e salva in x` | `Ask user for a number and store in x` |
| Conditional | `Se x è maggiore di 3` | `If x is greater than 3` |
| Alternative | `Altrimenti` | `Otherwise` |
| Counted loop | `Ripeti 3 volte` | `Repeat 3 times` |
| Condition loop | `Ripeti finché x è uguale a 3` | `Repeat until x equals 3` |
| While loop | `Mentre x < 5` | `While x < 5` |
| Function | `Funzione doppio(x)` | `Function double(x)` |
| Return | `Restituisci x * 2` | `Return x * 2` |
| List | `Crea una lista chiamata numeri` | `Create a list named numbers` |
| Append | `Aggiungi 4 a numeri` | `Add 4 to numbers` |
| End block | `Fine` | `End` |
| Local ICMP | `Scansiona la rete locale` | `Scan network` |

Quoted strings retain their original content. Blocks **currently require `Fine` or `End`**; indentation does not define blocks. Identifier names are currently ASCII, and case-sensitive.

See [GGUF Evaluation](docs/GGUF_EVALUATION.md), [Language Reference](docs/LANGUAGE_REFERENCE.md), [Multilingual guide](docs/MULTILINGUAL.md) and [Examples](examples). Spanish / French / German keyword support is **partial and experimental**, not at the same deterministic coverage level as IT/EN. The local model can attempt freer variants in all five languages.

## Mathematics

Native execution supports arithmetic `+ - * / % ^ **`, parentheses, unary signs, decimal values, and right-associative exponents; e.g. `2 ^ 3 ^ 2` is `512`, while `-2 ^ 2` is `-4`.

| Concept | Expression examples |
| --- | --- |
| Addition/subtraction | `2 plus 2`, `2 più 2`, `10 meno 3` |
| Multiplication/division | `5 per 4`, `7 diviso 2`, `3 times 4` |
| Remainder | `10 modulo 3` |
| Powers | `2 elevato a 5`, `pow(2,5)`, `potenza(2,5)` |
| Roots | `sqrt(81)`, `radice(81)`, `radice quadrata di 81`, `cbrt(27)` |
| Percentages | `15 per cento di 200`, `percent(15,200)`, `percentuale(15,200)` |
| More math | `abs`, `round`, `floor`, `ceil`, `ln`, `log10`, `exp`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `factorial`, `clamp`, `sign` |
| Lists | `somma([2,4,6])`, `media([2,4,6])`, `minimo([2,4,6])`, `massimo([2,4,6])` |
| Constants | `pi`, `euler` |

**Radians** are used for trigonometry; `log()` means natural logarithm. Invalid operations such as division by zero, negative square root and invalid logarithm domain raise runtime errors. All numbers currently use IEEE-754 `double`, not arbitrary precision or decimal financial arithmetic. [Full mathematical reference](docs/MATH.md).

## Less rigid syntax via a local LLM

The deterministic parser is the fast, inspectable default. With `--llm`, NatLang asks a local `llama.cpp` inference server **only if deterministic parsing fails**. With `--llm-all`, it tries to normalize every program, including prose statements that may be syntactically valid but meant differently. The model produces **canonical NatLang source**, then the same compiler parses, validates and emits C++; the executable does **not** depend on AI.

Start a compatible local model yourself:

```sh
llama-server -m /path/to/instruction-model.gguf --host 127.0.0.1 --port 8080 -c 4096
./build/natc examples/free_form_multilingual.nat --llm-all --show-normalized --keep-cpp -o demo
```

The model is **not bundled or benchmarked**. `llama.cpp` must be installed separately, and `curl` must be available. The endpoint is restricted to loopback. Always review the normalized program, especially before running code that accesses local files or networks. See [Local LLM](docs/LOCAL_LLM.md) and [Natural-language design](docs/NATURAL_LANGUAGE.md).

## Project documentation

- [Idea in italiano](docs/IDEA_IT.md) · [Vision](docs/VISION.md)
- [Architecture and shared IR](docs/ARCHITECTURE.md) · [IR details](docs/SEMANTIC_IR.md)
- [Language Reference](docs/LANGUAGE_REFERENCE.md) · [Multilingual](docs/MULTILINGUAL.md) · [Math](docs/MATH.md)
- [LLM integration](docs/LOCAL_LLM.md) · [Natural-language roadmap](docs/NATURAL_LANGUAGE.md) · [Roadmap](docs/ROADMAP.md)
- [Changelog](CHANGELOG.md) · [Contributing](CONTRIBUTING.md) · [Security](SECURITY.md)

## Tests

```sh
python3 -m unittest discover -s tests -v
```

The tests exercise native compilation and execution, IT/EN + selected ES/FR/DE programs, mathematical behavior and errors, input/output, lists, file operations and a **mock** `llama-server`. Mock protocol tests **do not demonstrate the accuracy of a real LLM**. GitHub Actions runs Linux and Windows jobs; platform-specific results must be checked on GitHub.

## Limitations and responsible use

- Arbitrary free-form prose, modules, classes, imports, GUI, robust general networking and complete static typing are **not** implemented. The statement IR is not yet a fully typed expression AST.
- Model translation can be plausible yet wrong. Never assume compiling proves semantic equivalence to the original natural-language request.
- `Scan network` is a bounded, ICMP-only scan of private LAN interfaces; an ICMP timeout does not prove a host is offline. Scan only networks and hosts you are authorized to inspect.
- Compiler paths and the chosen toolchain are trusted inputs; this prototype is **not** a secure sandbox for hostile source programs.

**License:** MIT — © 2026 Gabriele Viola and NatLang contributors. Contributions welcome.

**Build-speed tip:** use `--opt-level 0` for faster generated-program compilation during development; the default remains `--opt-level 2`. This only affects the host C++ compilation step.
