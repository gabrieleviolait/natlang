# Shared intermediate representation (v0.3)

The deterministic frontend turns source lines from recognized languages into `Node` records (`assign`, `print`, `input`, `if`, `repeat-times`, `function`, etc.). Generated expressions are parsed and checked by one shared expression compiler. `--emit-ir` exports the **statement-level** intermediate representation for tooling:

```sh
natc examples/bilingual_program.nat --emit-ir program.ir.json
natc examples/bilingual_program.nat --check --explain
```

Example IR fragment:

```json
[{"kind":"assign","line":1,"a":"x","b":"3","c":"","parameters":[],"body":[],"otherwise":[]}]
```

This statement IR records source expressions as strings (`a`, `b`, `c`), **not** typed expression AST nodes or stable versioned binary IR. It is inspectable debug output and can change between compiler versions. Italian, English and other keyword aliases map to the same `kind` records and use the same C++ backend. Exporting the IR does **not** prove the source's intended meaning; the optional local LLM normalization needs review.
