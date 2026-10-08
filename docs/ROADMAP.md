# Roadmap and research milestones

This is a proposed direction, **not a commitment or implemented feature list**.

| Version | Milestone | Acceptance criteria |
|---|---|---|
| v0.1 (current) | Deterministic English-like frontend, optional local LLM, C++20 native code generation | CLI + examples + integration tests |
| v0.2 | Typed expression AST, explicit IR, lexical scopes, source spans, definite-assignment checking | No unknown/uninitialized uses can slip silently into generated code; detailed diagnostic tests |
| v0.3 | Modules/imports, records/maps, native C FFI, standard library building blocks | Build a multi-file native command-line application without editing generated C++ |
| v0.4 | Local model benchmark, error/ambiguity UX, per-block normalization, caching | Publish reproducible accuracy/latency metrics and ambiguous-case outcomes |
| v0.5 | First-class GUI + networking + library APIs | Implement and test a small cross-platform application |
| v0.6+ | Multi-backend IR, incremental compilation, tooling, package management | Stable language spec and integration contracts |

## Priority backlog

- Split `natc.cpp` into lexing, parsing, semantic, codegen and CLI translation units.
- Replace shell calls with a safe subprocess abstraction.
- Adopt a full JSON parser and better structured local-model error handling.
- Add semantic tests for short-circuit logic, recursion depth, floating-point behavior, Unicode and nested scopes.
- Support meaningful source locations in emitted C++ diagnostics.
- Publish reproducible benchmark scripts and datasets, with false-positive (plausible but wrong) translation analysis.
- Cross-platform build & release automation, code signing guidance, and model distribution/licensing guidance.

## Contribution opportunities

Compiler frontend improvements, small-model evaluation, test programs, documentation translations and platform packaging are all welcome. Begin with [CONTRIBUTING.md](../CONTRIBUTING.md). Proposals should distinguish an *implemented behavior* from a *future language capability*.
