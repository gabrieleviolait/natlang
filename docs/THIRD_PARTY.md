# Third-party components in NatLang Studio Easy

The NatLang source code is MIT licensed. The Windows Easy packaging workflow downloads distinct components subject to their respective licenses:

- **llvm-mingw (Clang/LLVM, mingw-w64, compiler runtime):** https://github.com/mstorsjo/llvm-mingw , release `20260922` (UCRT x86_64). Components use multiple licenses, including LLVM Apache-2.0 WITH LLVM-exception and mingw-w64 licenses. Preserve the upstream `LICENSE*`, `COPYING*` and notices inside the toolchain; see the distribution's files.
- **llama.cpp CPU-only Windows runtime:** https://github.com/ggml-org/llama.cpp , upstream release chosen at packaging time; source distributed under MIT. Included binaries and supporting dependency licenses remain upstream's. The release identity is written to `ai/runtime-version.txt`.
- **Optional Qwen3-0.6B-Q4_K_M GGUF:** https://huggingface.co/tensorblock/Qwen_Qwen3-0.6B-GGUF (quantization). Model license is Apache-2.0; license and model card at source. Included only in the explicitly selected full-package build, otherwise downloaded by llama.cpp at first AI use.

No upstream trademarks or logos are asserted as NatLang property. The installer build runs through the public GitHub Actions workflow so users can inspect sources. For redistribution, verify the complete notices accompanying each pinned third-party release and the relevant model license.

The Windows packaging workflow copies llama.cpp's upstream MIT license into `ai/licenses/llama.cpp-LICENSE-MIT.txt`. Preserve this license file if redistributing the packaged executable. Toolchain license files remain inside `toolchain/`.
