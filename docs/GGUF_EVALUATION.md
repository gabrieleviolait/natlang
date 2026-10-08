# Local GGUF normalization and reproducible evaluation — NatLang v0.4

## What is actually implemented

NatLang has a **small-model-oriented, prompt-specialized** frontend. The GGUF weights run in the separately installed `llama-server`, not in the compiled NatLang program. Native executables remain independent of the model. The compiler:

1. Sends a user-provided source to a **loopback-only** OpenAI-compatible `llama.cpp` chat endpoint.
2. Selects `--llm-profile qwen3` (compact few-shot multilingual prompt, Qwen3 `/no_think`) or `generic`.
3. Requests a JSON-schema response containing exactly the canonical NatLang `program` string.
4. Parses this source via the same deterministic parser and validates the resulting IR/C++ generation.
5. If the candidate **fails validation**, optionally requests one correction with the actual compiler error (`--llm-attempts 1|2`, default 2). It does not retry candidates that compile but misinterpret the request.
6. Offers `--llm-preview` to inspect both the normalized program and shared IR without producing or executing binaries.

**Not implemented:** training/fine-tuning a model, an embedded GGUF inference runtime, general free-form understanding guarantees, semantic equivalence proof, native executable sandboxing or packaging a model. `--llm-all` can silently misinterpret valid source, so use preview and tests.

## Recommended models

- **Qwen3-0.6B, Q4_K_M (~484 MB)**: recommended first candidate for five-language evaluation; original model under Apache-2.0. GGUF repository: https://huggingface.co/tensorblock/Qwen_Qwen3-0.6B-GGUF .
- **SmolLM2-360M-Instruct, Q4_K_M (~271 MB)**: smaller comparison candidate, primarily English; Apache-2.0. GGUF repository: https://huggingface.co/tensorblock/SmolLM2-360M-Instruct-GGUF . Use `--llm-profile generic` initially.

The model GGUF must be downloaded by you; none are stored in this repository. The project's MIT license applies to NatLang source only; external GGUF/llama.cpp licensing remains independent.

## Windows setup (PowerShell)

Install llama.cpp from its official release or `winget install llama.cpp`. Make `llama-server` available on `PATH`.

```powershell
.\scripts\start_gguf.ps1 -Model qwen3
```

This binds to `127.0.0.1:8080`, downloads a Q4_K_M model if missing, and uses the server model alias `local-model` to match `natc` requests. Keep this window open. In a second terminal:

```powershell
.\build\Release\natc.exe examples\free_form_multilingual.nat --llm-preview
.\build\Release\natc.exe examples\free_form_multilingual.nat --llm-all --show-normalized --check
python benchmarks\evaluate.py --natc .\build\Release\natc.exe --mode gguf --model-label Qwen3-0.6B-Q4_K_M --report qwen3-results.json
```

Run the same benchmark in deterministic mode for comparison:

```powershell
python benchmarks\evaluate.py --natc .\build\Release\natc.exe --mode deterministic --report baseline.json
```

For the smaller comparison model, stop the first server, start `start_gguf.ps1 -Model smollm2`, then benchmark with `--profile generic --model-label SmolLM2-360M-Q4_K_M`.

## Linux and macOS

```bash
./scripts/start_gguf.sh qwen3
# in a second terminal:
./build/natc examples/free_form_multilingual.nat --llm-preview
python3 benchmarks/evaluate.py --natc ./build/natc --mode gguf --model-label Qwen3-0.6B-Q4_K_M --report qwen3-results.json
```

To use already downloaded weights: `./scripts/start_gguf.sh /path/model.gguf`. Set a different port via the scripts and `natc --llm-url http://127.0.0.1:PORT/v1/chat/completions` or `evaluate.py --url`.

## Benchmark design and limitations

`benchmarks/cases.jsonl`: **59 held-out illustrative test requests** (54 positive / 5 unsupported), covering five languages, mathematics, variables, control flow, input and lists. Each positive case includes a deterministic *canonical reference*. The evaluator runs `natc --emit-ir` for the reference, runs the chosen frontend for the source, strips only IR line-number metadata and tests **strict structural agreement**. Unsupported instructions are counted as successful only when the compiler rejects them. It also reports parse acceptance, median/mean/p95 per-request duration and grouped language counts. A syntactically different but behaviorally equivalent result can be incorrectly scored as a mismatch; on the other hand, equal IR alone does not prove a full application is correct.

**Baseline measured here on a container with no LLM connected:** deterministic mode 5/59 strict matches, all 5 being correctly rejected unsupported requests; only 2/54 positive requests were syntactically accepted and their raw IR differed from the canonical strict references. This is an intentionally difficult, free-form dataset, not a general accuracy benchmark of NatLang or a GGUF. Latency measurements are hardware dependent. **No real GGUF inference was executed in the preparation environment**, so there are **no real model accuracy, inference speed or memory figures** reported. The six local mocked-server integration tests exercise request format, retries, IR preview, fallback and refusal; they are **not** evidence of Qwen3/SmolLM2 quality.

To evaluate a GGUF fairly, record the exact quantization file/hash, llama.cpp version, hardware (CPU, GPU, RAM), warm-up strategy, model label, profile, test date, memory peak and use the same 59 examples and timeout. Ideally augment this strict-IR proxy with executable-output comparison and human reviews of semantic fidelity. Avoid placing the held-out benchmark inputs into training data or the prompt examples.

## Safeguards

- Only `http://127.0.0.1:<port>` and `http://localhost:<port>` endpoints are allowed; no cloud model fallback.
- Only generated NatLang source is accepted, never raw generated C++ or shell snippets.
- Canonical output must pass parser, variable/function and expression validation before native emission.
- Rejected outputs cause an explicit compile failure (and at most one correction request).
- A **valid** yet semantically incorrect program can still compile. `--llm-preview`, `--show-normalized` and application tests are essential for consequential programs.
- Generated executables can access local files/network according to their native runtime; run untrusted source with limited privileges.

A truly **specialized model** would additionally need curated training examples, a separate validation split, optional supervised fine-tuning/LoRA and new quantization to GGUF. This release is **prompt specialization + evaluation infrastructure**, not a new trained model.
