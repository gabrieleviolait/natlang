# Changelog

## 0.1.0 — 2026-10-08

Initial experimental source distribution:

- C++20 `natc` frontend for a restricted English-like `.nat` language.
- Native code generation via host C++ compiler, with small embedded `nat::Value` runtime.
- Variables, expressions, conditions, loops, functions, lists, text/numeric input, and files.
- Optional loopback llama-server normalization adapter (model not included).
- Example programs, end-to-end tests, platform build scripts, architecture, vision and language documentation.

**Limitations:** not syntax-free, not feature-complete general-purpose programming, no integrated/fine-tuned model. See [README.md](README.md).
