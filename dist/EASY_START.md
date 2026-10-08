# NatLang Studio Easy v0.5.1 — one installer or portable ZIP (Windows 10/11 x64)

NatLang Studio Easy is a *desktop frontend* to the same open-source C++20 `natc` compiler. You can use it without Git, CMake, Visual Studio, Python, or a cloud service. The GitHub Actions package includes a portable **llvm-mingw Clang C++ toolchain**, so native compilation does not require a separate SDK installation.

## Start in 3 steps

1. Visit the public [Releases page](https://github.com/gabrieleviolait/natlang/releases) and download the **Easy Setup EXE** or **Easy ZIP**, plus optional SHA256SUMS. If no release assets are present yet, open [Actions → Windows packages](https://github.com/gabrieleviolait/natlang/actions/workflows/easy-windows.yml) and check for a successful build. GitHub Actions artifacts require GitHub sign-in; Releases are public.
2. Run `NatLang-Studio-Easy-Setup.exe` (per-user installer, no admin rights required), **or** extract `NatLang-Studio-Easy-Windows-x64.zip` and launch `natlang-studio.exe`.
3. Type `Mostra 2 più 2`; click **Esegui** to compile into a native executable and see `4` in the output area. Click **Salva** to keep your `.nat` file.

**Buttons:** Nuovo (new), Apri (open), Salva (save), Verifica (parser check), Compila (native `.exe`), Esegui (compile + run, output inside the app), Anteprima AI (AI normalization only; no generated code execution), Avvia AI (start local llama-server), Invia input (send text to a program currently waiting for stdin).

## Optional offline AI

- Click **Avvia AI**. The included CPU-only `llama-server.exe` runs *locally* at `127.0.0.1:8080`, independent of cloud APIs. Its window stays open while you are using AI.
- For standard Easy packages, on first launch llama.cpp downloads a quantized `Qwen3-0.6B Q4_K_M` model (~484 MB) from Hugging Face. This download needs internet access. Later launches can use the existing llama.cpp cache.
- The optional **Full** build includes `ai/models/Qwen3-0.6B-Q4_K_M.gguf`, so AI can start without downloading model weights. Both packages contain the C++ toolchain and llama.cpp runtime.
- Click **Anteprima AI** to see a model-assisted rewrite and the compiler IR before generating or executing a program. **Review and test translations**. Natural-language equivalence is not guaranteed.

## Troubleshooting

- **Windows SmartScreen:** Windows may warn about unsigned community-developed binaries. Verify the publisher and build provenance on GitHub. Do not disable system protection globally.
- **Executable or toolchain missing:** Use the full Easy artifact/installer, not only `natlang-studio.exe`; preserve `toolchain/` and `natc.exe` beside it.
- **AI fails to connect:** Start AI first and wait for `llama-server` to load the model. Keep its PowerShell window running. For a custom local model, run `scripts/start_gguf.ps1 -GGUFPath C:\path\model.gguf`.
- **Slow native builds:** Studio uses `--opt-level 0` for fast iteration. The resulting `.exe` is native, but compilation still takes time. For optimized releases use `natc source.nat --opt-level 2`.
- **Security:** Running `.nat` can access local files or ping permitted networks. NatLang is not a sandbox. Only run trusted code and review AI translations.

## Licenses / provenance

NatLang MIT; llvm-mingw, LLVM/Clang, MinGW-w64, llama.cpp and GGUF have their **own** licenses and notices. See `docs/THIRD_PARTY.md`. Third-party binaries are fetched by the Windows packaging job from upstream projects; they are not committed to the source repository. Models are not redistributed unless the optional Full build is explicitly selected. Build scripts reference the upstream release and model IDs so updates are auditable.
