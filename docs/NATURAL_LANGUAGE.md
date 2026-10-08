# Toward syntax-light natural-language programming

## What exists today

There are **two frontends**, not two backends:

1. **Fast deterministic frontend:** expressions and recognized English/Italian (and selected ES/FR/DE) statements map to the shared NatLang statement IR. This needs no model, and errors are reproducible.
2. **Local LLM normalization (opt-in):** `--llm` handles errors by asking a locally running instruction model to rewrite the program as canonical NatLang; `--llm-all` normalizes proactively. The same deterministic parser, expression checker and C++ generator then run.

**No model is shipped or fine-tuned.** Accuracy of a 0.3B/0.6B model, multilingual semantic preservation, latency, memory usage and effectiveness on long scripts remain open evaluation tasks. Grammar-constrained JSON output only validates response *shape*, not program meaning.

## Intended architecture

```text
Source: IT / EN / ES / FR / DE (or mixture)
      |
      +--> deterministic normalization -----------------+
      |                                                 |
      +--> optional local LLM --> canonical .nat --------+--> statement IR
                                                            |
                                                      expression checks
                                                            |
                                                    C++20 code generation
                                                            |
                                                     host C++ compiler
                                                            |
                                                     native executable
```

## Essential accuracy work before claiming 'no syntax rules'

- Build a reviewed corpus with sentence-level and program-level expected behavior, including adversarial ambiguities.
- Create a genuinely typed expression AST and stable semantic IR, with source-span diagnostics and definite-assignment checks.
- Add a model-specific benchmark: exact IR match, executable-output equivalence, ambiguity flags, runtime and memory measurements, separated by language.
- Introduce IR-based compilation caches and safe incremental normalization when the same source fragment has been reviewed.
- Add a review/approval mode for ambiguous changes and explicit prompts when assumptions cannot be inferred.
- Evaluate GGUF local models and optional fine-tuning independently from the host compiler.

This is a credible experimental route to a syntax-light language, not a claim that arbitrary prose can be compiled correctly today.
