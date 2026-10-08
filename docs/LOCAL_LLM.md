# Optional local LLM normalization (NatLang v0.3)

NatLang uses a small LLM **only as an optional natural-language frontend**; it does not generate arbitrary C++ from an unreviewed user request. Inference is local through a `llama.cpp` compatible chat-completions endpoint. This repo does **not** include a GGUF file, model training pipeline, or a model-quality guarantee.

## Setup

Install `llama.cpp` / `llama-server` separately, choose an instruction-tuned GGUF whose license you have checked, and start:

```sh
llama-server -m /path/to/instruct-model.gguf --host 127.0.0.1 --port 8080 -c 4096
```

Then:

```sh
natc examples/free_form_multilingual.nat --llm-all --show-normalized --keep-cpp -o demo
natc your_program.nat --llm --show-normalized -o app
natc your_program.nat --llm-url http://127.0.0.1:8081/v1/chat/completions --llm -o app
```

- `--llm`: fallback only when deterministic compilation fails.
- `--llm-all`: always normalize, including potentially misleading statements that happen to parse.
- `--show-normalized`: print the model's canonical program, for semantic review.
- `--emit-ir`: inspect the validated shared statement IR (use in a separate compilation command).
- `--keep-cpp` / `--emit-cpp`: inspect the native C++ output.

The model must return an OpenAI-compatible response with JSON-encoded `choices[0].message.content` containing JSON `{"program":"canonical NatLang source"}`. NatLang requests a schema-constrained JSON response and `temperature=0`. This does **not** guarantee correctness or deterministic inference across backends. `curl` is required, URLs must point to localhost/127.0.0.1, and there is no automatic cloud fallback.

## Multilingual scope

The normalization prompt supports **Italian, English, Spanish, French and German** instructions and attempts to map them to the **same** canonical NatLang syntax. Deterministic IT/EN works without any model; the listed additional languages currently have only a subset of rule-based commands. General prose, unrecognized spellings, compound requests and ambiguous meaning require the model and may fail.

## Security and evaluation

LLM output is parsed and name-checked, but semantic fidelity to the natural-language request is **not verified**. Always inspect code and test program outputs before running important tasks. Compile untrusted programs with minimal OS permissions and in a sandbox. Free-form requests can contain ambiguous demands, invented variable names or unsupported concepts. The project uses a fake local server for integration tests; it **does not** yet benchmark real model accuracy, memory, latency or quantized weights.

Recommended next step: curate examples with verified AST and native outputs, and independently benchmark candidate local models before any fine-tuning or bundled release.

## v0.4 enhancement

Use `--llm-preview` for **model-assisted normalization without generating or running executables**, `--llm-profile qwen3|generic` to select a local prompt specialization, and `--llm-attempts 1|2` to control at most one retry when the candidate fails NatLang compilation. Default profile is `qwen3`; the generic profile omits the extra small-model examples. Both still require an external local llama.cpp-compatible server.

See [GGUF setup and evaluation](GGUF_EVALUATION.md) for reproducible commands, real-vs-mock testing separation, candidate model licenses, and benchmark caveats.
