# Roadmap — NatLang

**Current: v0.3.0 experimental milestone.** The intent is a general-purpose language with native output, language-independent semantics and an optional tiny local LLM. This is not a completed general-purpose language.

1. **v0.4 — Real semantic foundation:** typed expression AST, static typing or inference, lexical scopes, definite assignment, source-span errors, stable versioned IR and better module/function semantics.
2. **v0.5 — Syntax-light language evaluation:** multilingual parallel datasets, model comparisons and measurable semantic correctness; normalization cache; ambiguity/approval workflow; local GGUF model packaging subject to separate model licenses and footprint constraints.
3. **v0.6 — General-purpose foundations:** dictionaries, modules, error handling, interfaces to safe native libraries, improved standard library and structured networking (with authorization controls).
4. **Later:** ergonomic IDE/editor, incremental compilation, fuller internationalization, GUI tools, async/concurrency, optional LLVM IR backend and package distribution.

Decisions are guided by native binary quality, portability, memory/latency, diagnostics, and measured correctness of natural-language translation — not by claiming zero syntax restrictions prematurely.
