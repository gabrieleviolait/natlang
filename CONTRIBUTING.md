# Contributing to NatLang

Thanks for helping explore natural-language native compilation. NatLang is experimental: reproducible bug reports, small failing `.nat` programs, runtime tests and docs improvements are especially valuable.

## Development setup

Requires CMake 3.16+, a C++20 compiler and Python 3 for tests.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
python -m unittest discover -s tests -v
```

On Windows with Visual Studio Build Tools, configure/build from a Developer PowerShell. The compiler executable may be under `build/Release/natc.exe`; use `NATC` to point the Python test suite to it.

## Proposing changes

1. Open an issue describing a supported or proposed behavior. For new phrase patterns, include natural input, expected canonical instructions and any ambiguity.
2. Add executable tests when modifying the parser, emitter or runtime.
3. Keep changes local-first and preserve the boundary between model normalization and deterministic compiler stages.
4. Describe compatibility or portability implications in the pull request.
5. Do not claim general-purpose functionality or model accuracy that has not been measured.

There is no published ABI/API stability guarantee before v1.0. Use the MIT license for contributions; do not include proprietary code or model weights you do not have rights to redistribute.
