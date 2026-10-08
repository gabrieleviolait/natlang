# NatLang: vision and product concept

**Status:** research prototype, not a completed general-purpose language.

## One-sentence pitch

NatLang aims to let people **describe the behavior of a real program in ordinary language** and compile that description into a **standalone native executable**, without shipping an AI model with the program.

## Why build this?

Traditional programming languages require users to learn precise syntax before expressing an algorithm. Contemporary AI code assistants accept natural-language prompts but generally generate and edit code in a separate conventional language, often with a cloud model in the loop. NatLang explores a different boundary: **the human-written source file is the primary language**, and the language runtime and compiler own the executable artifact.

The long-term goal is not a wrapper around a chatbot or a collection of desktop automation macros. It is a programming environment capable of supporting algorithms, functions, data structures, files, GUI programs, networking, libraries, and native application development, with a natural-language frontend.

## Core principles

1. **Natural expression, strict execution.** Source should be flexible; the normalized intermediate representation must be precise.
2. **Compile once, run independently.** The generated application must not need an LLM, an API key, or the NatLang compiler at runtime.
3. **Local-first.** Deterministic parsing and local model inference should work offline; no mandatory remote service.
4. **Small AI surface.** An optional model translates intentions to a constrained language, rather than generating unrestricted C++.
5. **Inspectable output.** Users can view the normalized program and generated C++, and validate what will run.
6. **Failure over invention.** An unsupported or ambiguous instruction should fail explicitly instead of silently changing the program's meaning.
7. **Open toolchain.** C++20 implementation, readable runtime, testable compiler components, permissive license.
8. **Portable architecture.** Host builds can target different operating systems; cross-compilation is a future goal, not a v0.1 feature.

## Example target experience

A possible future program (illustrative vision, **not all currently supported**):

```text
Create an application called PhotoSorter
Ask the user to select a folder
Find all JPEG and PNG images in that folder
Group them by the month they were created
Create a folder for each month
Move each image to its corresponding folder
Show a progress bar and a summary
```

The compiler would translate that description into typed operations, ask about unresolved choices such as whether to overwrite files, select appropriate standard-library modules, and emit native code.

A supported v0.1 program looks like:

```text
Ask the user for 5 numbers and store them in values
Set biggest to max(values)
Set mean to average(values)
Show "Maximum: " + biggest
Show "Average: " + mean
```

The distinction between **vision** and **implemented functionality** is important: NatLang v0.1 still has a restricted English-like grammar and requires explicit `End` delimiters.

## Proposed language architecture

```text
.nat human source
     |
     +--> deterministic phrase frontend --------+
     |                                           |
     +--> optional local LLM normalization ------+ 
                                                 |
                                      canonical NatLang text
                                                 |
                                      structural parser / AST
                                                 |
                                      symbol / expression checks
                                                 |
                                      C++20 source generation
                                                 |
                                      host compiler (Clang/GCC/MSVC)
                                                 |
                                         standalone binary
```

**Future** versions should replace the current code-generation-heavy validation path with a language-independent typed semantic IR, independent optimizations, modular backends, and first-class ambiguity resolution.

## What the AI is (and is not)

AI is an **optional compiler frontend**, not a requirement for execution. It may normalize alternative phrasing into supported operations; it is not trusted to execute shell commands, invent libraries, or bypass validation. Structural validity does not prove semantic equivalence, so output should be reviewed and tested.

## What success looks like

Before claiming general-purpose capability, measurable milestones should include:

- A broad suite of deterministic language tests on all supported platforms.
- A benchmark dataset of human phrasing paired with expected canonical ASTs, including ambiguous examples.
- Semantic-preservation scoring, not merely the percentage of programs that compile.
- A typed IR and a documented standard library with usable file, network, GUI, and module support.
- Repeatability across model versions and machines.
- Developer experience: understandable errors, source mapping, debugger integration, test runner, and package support.

## Positioning

Natural-language programming has predecessors and adjacent systems (for example Inform 7, block programming environments, desktop automation systems, and AI code assistants). NatLang does **not** claim to invent natural-language programming. Its intended combination is a **local, lightweight, inspectable natural-language compiler that outputs native applications without model dependence at runtime**.

## Non-goals for v0.1

- Universal comprehension of arbitrary English or Italian.
- Automatically generating safe or correct algorithms from underspecified requirements.
- Compiling arbitrary third-party C++/Python modules.
- Autonomous screen recognition, GUI creation, HTTP, sockets, threads, or system-wide computer control.
- Competitive optimization versus industrial C++ compilers.

See [architecture](ARCHITECTURE.md), [language reference](LANGUAGE_REFERENCE.md), and [roadmap](ROADMAP.md).
