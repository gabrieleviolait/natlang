# Optional local LLM normalization

## Goal

Use a small language model only as a **natural-language frontend**. The output is constrained to canonical NatLang text, not arbitrary C++ or executable shell commands. The normal compiler parses and validates that text before emitting machine code.

## Current implementation

`natc` currently calls an OpenAI-compatible `/v1/chat/completions` HTTP endpoint on **127.0.0.1 or localhost only**, with a JSON-schema response format requiring `{"program":"..."}`. It relies on the local `curl` executable and a previously started `llama.cpp` `llama-server` instance.

```bash
llama-server -m /path/to/instruct-model.gguf --host 127.0.0.1 --port 8080 -c 4096
natc examples/free_form_llm.nat --llm --show-normalized --keep-cpp -o free_form
```

Use `--llm` for fallback-only normalization, `--llm-all` to always normalize, or `--llm-url http://127.0.0.1:8081/v1/chat/completions` for a custom loopback port. **No model files are included** and no particular model is known to be reliable for this task yet.

### Prompt protocol

The system prompt enumerates supported NatLang constructs and asks the model to preserve behavior, write literal strings in quotes, use `End`, and leave unsupported instructions unmodified to trigger a visible parser failure. The request uses `temperature=0`; that does not imply perfect determinism or semantic accuracy.

Example model response body:

```json
{
  "choices": [{
    "message": {
      "content": "{\"program\":\"Set x to 7\\nShow x + 3\"}"
    }
  }]
}
```

## Trust boundary

A model can return a syntactically valid but **incorrect program**. The current compiler checks grammar and referenced names, **not equivalence to the user's intent**. Keep `--show-normalized` on when experimenting. Review generated C++ with `--emit-cpp`/`--keep-cpp`, run tests, and do not compile untrusted natural-language prompts with important file privileges.

The adapter is not a secure sandbox and does not provide formal verification. Loopback-only requests prevent accidentally targeting a cloud endpoint, but prompts still leave `natc` for the chosen local server process.

## Benchmark and training plan (future)

1. Create a corpus of natural requests with canonical NatLang examples and expected observable behavior.
2. Split by *task family* (not random near-duplicates), and include ambiguous/invalid requests.
3. Evaluate exact AST match, execution-based equivalence, refusal/uncertainty accuracy and latency.
4. Compare deterministic-only, tiny models (~0.3B–0.6B), somewhat larger local models, and potential fine-tuning.
5. Add per-block compilation, semantic-cache keys and an inspectable ambiguity report before claiming syntax freedom.

The important optimization target is **semantic correctness per unit of model memory and time**, not model size alone.
