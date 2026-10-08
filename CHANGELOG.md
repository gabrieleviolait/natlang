# Changelog

## 0.2.0 — 2026-10-08

- Bare mathematical expressions now print their result: `2 + 2`, `2 plus 2`, `2 piu 2` and `2 più 2` produce `4`.
- `natc --eval "2 plus 2"` compiles and immediately executes a native one-line program.
- Interactive prompts: `Ask user`, `Ask user "Question? " and store in name`, `Chiedi all'utente`, `Chiedi ad utente un numero e salva in eta`.
- `Scan IP <IPv4>`, `Scan this ip`, `Scan network`, `ping(address)`: IPv4-only ICMP reachability with strict validation and limited private-LAN discovery.
- Cross-platform interface discovery (Windows, Linux, macOS source support), bounded concurrent ping probes, validation against shell injection; portable Windows runtime linkage added for GCC/Clang.
- New examples and integration tests; prior English input forms remain backward compatible.

**Limitations:** network reachability is ICMP-only, and blocking ICMP can look like nonresponse; LAN scan checks at most two local /24 slices. Features remain experimental.


## 0.1.0 — 2026-10-08

Initial experimental source distribution:

- C++20 `natc` frontend for a restricted English-like `.nat` language.
- Native code generation via host C++ compiler, with small embedded `nat::Value` runtime.
- Variables, expressions, conditions, loops, functions, lists, text/numeric input, and files.
- Optional loopback llama-server normalization adapter (model not included).
- Example programs, end-to-end tests, platform build scripts, architecture, vision and language documentation.

**Limitations:** not syntax-free, not feature-complete general-purpose programming, no integrated/fine-tuned model. See [README.md](README.md).
