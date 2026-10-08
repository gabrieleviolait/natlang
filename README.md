# NatLang — Native Natural-Language Compiler (experimental v0.2)

An **actual C++20 implementation** of a small English-like programming language. `natc` translates `.nat` source into standalone C++20 translation units, then uses the installed Clang/GCC/MSVC toolchain to build a real native binary. The generated programs **never call an LLM**. A tiny local model can optionally rewrite unfamiliar English into NatLang's supported form.

> **Honest scope:** This is a working, limited MVP, **not** yet a universal, syntax-free, general-purpose programming language. There is no fine-tuned model or bundled GGUF. The runtime has dynamic `Value` types rather than a complete static type system; it does not yet support GUI programming, general-purpose networking (only bounded IPv4 LAN ICMP discovery), object-oriented classes, native library imports, asynchronous tasks, or a full standard library. LLM results are not semantically guaranteed correct.

## Documentation and project idea

- **[Vision and original concept](docs/VISION.md)** — motivation, long-term general-purpose goal and what distinguishes NatLang.
- **[Idea progettuale (italiano)](docs/IDEA_IT.md)** — obiettivi, scelte architetturali e stato reale del prototipo.
- **[Architecture](docs/ARCHITECTURE.md)** — concrete compilation pipeline, implementation layout and design constraints.
- **[Language reference](docs/LANGUAGE_REFERENCE.md)** — exact v0.1 statements, expressions and limitations.
- **[Local LLM integration](docs/LOCAL_LLM.md)** — optional llama.cpp server, API shape, limitations and evaluation approach.
- **[Roadmap](docs/ROADMAP.md)** — proposed milestones, not promises.
- **[Contributing](CONTRIBUTING.md)** · **[Security](SECURITY.md)** · **[Changelog](CHANGELOG.md)** · **[License](LICENSE)**

## 1. Windows setup

Install Visual Studio 2022 Build Tools with **Desktop development with C++** and CMake (or equivalent Clang/MinGW-w64 toolchain). Open **Developer PowerShell for VS 2022**, extract this repository, and run:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\natc.exe examples\countdown.nat -o countdown.exe --compiler cl
.\countdown.exe
```

Or run `powershell -ExecutionPolicy Bypass -File scripts\build_windows.ps1` from the Developer PowerShell.

For a Ninja/MinGW build, the executable may instead be at `.\build\natc.exe`, and select `--compiler clang++` or `--compiler g++`.

## 2. Linux/macOS setup

With CMake 3.16+, a C++20 compiler, and optionally curl:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/natc examples/countdown.nat -o countdown
./countdown
```

Or `bash scripts/build_linux.sh`. The compiler binary itself has **no dependency on Python**; Python is needed only to run the integration tests.

## 3. Try it immediately: calculator and user input

You can run a single natural expression **without creating a file**:

```powershell
natc --eval "2 + 2"          # prints 4
natc --eval "2 plus 2"       # prints 4
natc --eval "2 piu 2"        # prints 4
```

Inside `.nat` files, **bare expressions are printed**. Both English and some Italian input forms work:

```text
2 + 2
2 plus 2
Ask user "Your name? " and store in name
Show "Hello, " + name
Chiedi all'utente un numero e salva in eta
Mostra eta + 1
```

`Ask user` and `Chiedi all'utente` alone read a line of text into the built-in variable `answer`. They prompt at runtime, like Python's `input()`. Numeric input uses a number conversion and rejects invalid values.

### IPv4 ICMP network discovery

```text
Scan IP 192.168.1.1
Scan this ip 192.168.1.10
Scan this ip
Scan network
```

`Scan this ip` without an address prompts for one. You may also set an IP string in a variable and use `Scan IP target`, or call `ping(target)` inside an expression. A single-IP scan is a one-shot ICMP ping; `Scan network` enumerates up to **two** active private LAN IPv4 interface `/24` slices and checks at most 508 host addresses with up to 16 workers. There is no port scan or exploit detection. Hosts that block ping appear as **no ICMP response**, not conclusively offline. The system `ping` executable must be installed/available, and environment permissions may prevent ICMP even for localhost. Network discovery only runs when the generated executable is executed. **Only scan networks and hosts you own or are authorized to test.**

## 4. Example .nat program

```text
Set counter to 5
Repeat until counter is 0
    Show counter
    Decrease counter by 1
End
Show "Done!"
```

Usage:

```sh
natc program.nat -o app                 # native binary
natc program.nat --emit-cpp             # standalone .generated.cpp
natc program.nat --check --explain      # parse and validate only
natc program.nat -o app --keep-cpp      # keep generated C++
```

**Note:** Code blocks still require `End`. Whitespace indentation is only visual. Unquoted words are variable names. Text constants must use quotes. Statements are case-insensitive; variable names are case-sensitive, ASCII identifiers. The complete, genuinely syntax-free goal remains future work.

## 5. Currently supported features

| Category | Sample phrase |
| --- | --- |
| Variables | `Set price to 20`, `Remember name as "Alex"` |
| Arithmetic | `Set total to price * 1.2`, `Increase total by 5`, `2 plus 2` → `4` |
| Output | `Show "Total: " + total` |
| Input | `Ask for a number and store it in answer`, `Ask user`, `Chiedi all'utente un numero e salva in eta` |
| Multiple inputs | `Ask for 5 numbers and store them in values` |
| Conditions | `If answer is greater than 10` / `Otherwise` / `End` |
| Loops | `Repeat until answer equals 10` / `End` |
| Counted loops | `Repeat 3 times` / `End` |
| Other loops | `While answer < 10` / `End` |
| Functions | `Define a function called square with parameter n` / `Return n * n` / `End` |
| Lists | `Create a list named values`, `Add 3 to values`, `Show [1, 2, 3]` |
| List operations | `sum(values)`, `average(values)`, `min(values)`, `max(values)`, `length(values)` |
| Files | `Save total to file "result.txt"`, `Load file "result.txt" into contents` |
| IPv4 discovery | `Scan network`, `Scan this ip`, `Scan IP 192.168.1.1`, `ping(target)` |
| Flow control | `Break`, `Continue`, `Return` |
| Logic | `and`, `or`, `not`, `==`, `!=`, `>=`, `<=`, `is`, `equals`, `greater than` |

Literal strings preserve their contents during phrase matching. Invalid or unknown variable/function references are rejected. Variables used before assignment fail at runtime. A runtime error also occurs when an operation expects a different dynamic type.

## 6. Optional tiny LLM (100% local)

The compiler doesn't require a model for its built-in natural phrases; free-form multilingual requests remain outside the deterministic subset. To handle freer English, start **llama.cpp `llama-server`** with an instruction-tuned GGUF (for example an experimental 0.6B-class model):

```sh
llama-server -m /path/to/model.gguf --host 127.0.0.1 --port 8080 -c 4096
```

Then compile:

```sh
natc examples/free_form_llm.nat --llm --show-normalized -o demo
```

`--llm` tries the deterministic translator first, calling the local server only if it fails. `--llm-all` always requests normalization, even when the deterministic parser succeeds. `--llm-url http://127.0.0.1:8080/v1/chat/completions` points to a different local port.

The adapter uses the **OpenAI-style chat-completions endpoint** and requests a schema-constrained JSON object of shape `{"program":"..."}`. It calls local `curl` and assumes `llama-server` is already installed and running. There is no remote inference and the compiler rejects non-loopback server URLs. **Model output is parsed and must pass deterministic validation**, but this does not prove that the model preserved the intended meaning. Always inspect generated/normalized code, especially for consequential actions. Very small models may be too inaccurate; larger models or task-specific fine-tuning may be needed. GPU is not mandatory for small GGUF models.

### Example: human-language-to-native pipeline

```text
free English -> local LLM (optional) -> canonical lines
       -> structural AST -> expression parser & name checks
       -> standalone C++20 -> clang++/g++/cl -> native executable
```

The host compiler is a separate dependency. `natc` currently uses **C++ code generation**, not a home-built assembler, optimizer, or direct LLVM IR backend.

## 7. Tests

```sh
python -m unittest discover -s tests -v
```

Uses the built `build/natc` and a host C++ compiler. Includes end-to-end generated executable tests, syntax errors, loops, recursion, file/lists/input, and a fake llama-server that verifies the LLM adapter protocol **without testing any real model quality**.

## 8. Practical roadmap

1. **v0.3:** Expression AST nodes, lexical scopes, definite-assignment checks, structured diagnostics and immutable semantic IR.
2. **v0.4:** Static type inference, modules, maps/dictionaries, error handling, packages and safer process execution.
3. **v0.5:** Language-model evaluation dataset, normalization caching, more multilingual expressions and ambiguity reports.
4. **Later:** GUI, general networking, async/concurrency, richer library ecosystem and optional LLVM IR backend.

## Security and trust

- Executing compiled programs can read/write files based on `.nat` instructions. Run unknown sources in a sandbox.
- An LLM can misunderstand valid English even when its normalized result compiles; semantic equivalence is not guaranteed.
- `natc` currently runs the selected C++ compiler through a system shell; treat the compiler binary, command-line arguments, filesystem paths, and local server as trusted inputs.
- Only loopback HTTP endpoints are accepted for LLM normalization. Local prompts are transmitted to the local server; no external model/provider is required.

License: MIT (see LICENSE). Prototype designed for further experimentation and contributions.
Project initiator: Gabriele Viola. Contributions welcome under the MIT license.
