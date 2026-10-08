# Changelog

## v0.5.1 — Windows packaging and public download pipeline (2026-10-08)

- Automatically publish installer/portable ZIP and SHA256 checksums to GitHub Releases after a successful Windows packaging workflow.
- Verify compiled programs using the bundled LLVM-MinGW toolchain before packaging.
- Pin llama.cpp build and keep provenance/version information; optional GGUF Full package remains selectable by manual workflow dispatch.
- Clarify download instructions and release verification in README and Easy Start guide.



## v0.5.0 — NatLang Studio Easy (2026-10-08)

- Added dependency-free Windows Win32 editor (open/save, check/build/run, stdin input, stop, LLM preview and AI launch).
- Added a GitHub Actions packaging workflow for Windows x64 portable ZIP and one-file Inno Setup installer. Package contains NatLang compiler, llvm-mingw toolchain, llama.cpp CPU runtime and examples.
- Added optional Full package mode with local Qwen3 GGUF model; the standard mode downloads model weights on first AI launch.
- Added Studio documentation, license notices and distribution checks.
- Important: distribution job must complete successfully before Windows packages can be considered tested or downloadable.


## v0.3.0 — Multilingual and mathematical language (2026-10-08)

- Expanded the same C++20 compiler frontend with richer **Italian and English** constructs, comparisons, list/file statements and math expressions.
- Experimental basic statement vocabulary in **Spanish, French and German**, with a shared statement IR and no separate backend per language.
- Mathematics: `^`, `**` (right-associative), percentage of amount, roots, absolute value, rounding, log/exp, trigonometry, factorial, range clipping and Italian builtin aliases.
- Expression synonyms: `2 plus 2`, `2 più 2`, `5 per 4`, `7 diviso 2`, `2 elevato a 5`, `15 per cento di 200` and more.
- Additional runtime checks for invalid exponent domains, square roots, logarithms, factorials and other non-finite mathematical results.
- `--emit-ir` outputs inspectable statement-level JSON; all recognized languages share the same IR and C++ code generator.
- Expanded the local LLM normalization prompt for five input languages; no model included, quality is model-dependent.
- Added multilingual/math examples, reference guides and end-to-end tests.
- Research prototype: not a universal syntax-free or fully fluent five-language compiler.

## v0.2.0

- Native single-expression evaluation (`--eval`), direct expression statements and basic word arithmetic.
- Interactive Italian/English text and number prompts.
- Bounded local private IPv4 ICMP discovery and explicit single-IP probing.

## v0.1.0

- First native C++20 compiler prototype with statements, expressions, control flow, functions, lists and files.
- Local LLM adapter and integration test harness.
