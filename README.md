# NatLang — Native Natural-Language Compiler

**v0.5.0 | Windows Studio Easy option · Multilingual frontend · native C++20 output · math · optional local GGUF frontend + benchmark**

NatLang is an experimental programming-language compiler. Write programs using readable sentences in **Italian or English**, mix languages inside a file, and build a real native executable through C++20. Common Spanish, French and German statements are recognized, while an **optional, user-provided local LLM** can try to normalize less structured multilingual instructions.

**Important:** This is a functional research prototype, **not** a fully syntax-free, universal natural-language compiler. No GGUF/model weights are included, and real quantized-model accuracy has not been validated. The deterministic frontend recognizes an explicit, documented subset. AI-generated normalized programs need review.

## Installazione completa da zero / Complete setup from scratch (Italiano)

> **Percorso consigliato:** Windows 10/11 a 64 bit, PowerShell e Visual Studio Build Tools 2022. Non servono un modello AI, Python o una GPU per compilare ed eseguire programmi `.nat` scritti con i comandi già supportati. **C++ e CMake, invece, sono necessari:** la v0.4.0 distribuisce i sorgenti del compilatore, non un `natc.exe` precompilato. Il frontend GGUF è facoltativo.
>
> Tutti i comandi dei punti **2–9** vanno eseguiti nella cartella principale di `natlang`, tranne dove specificato. Copia solo il contenuto dei blocchi di codice, senza i simboli del prompt del terminale.

### 1. Prerequisiti: cosa installare e cosa no

| Componente | Obbligatorio? | Serve per |
| --- | --- | --- |
| **Git** (oppure download ZIP da GitHub) | Uno dei due | Scaricare il progetto |
| **CMake 3.16+** | Sì | Costruire `natc` |
| **Compilatore C++20** (MSVC, GCC, Clang) | Sì | Costruire `natc` **e** compilare i programmi `.nat` |
| **PowerShell / terminale** | Sì | Lanciare i comandi |
| **llama.cpp / `llama-server`** | Solo per AI | Comprendere istruzioni più libere |
| **Modello istruito GGUF** (es. Qwen3-0.6B) | Solo per AI | Inferenza locale; i pesi non sono inclusi |
| **`curl`** | Solo per AI | Collegare `natc` al server locale |
| **Python 3** | Solo per i benchmark e test | Eseguire `benchmarks/evaluate.py` e la suite `unittest` |

### 2. Windows: installare Git, CMake e compilatore C++

Apri **PowerShell** dal menu Start e verifica se `winget` è disponibile:

```powershell
winget --version
```

Se `winget` non viene trovato, installa **App Installer** dal Microsoft Store oppure scarica Git e CMake dai siti ufficiali indicati sotto.

```powershell
winget install --id Git.Git --exact
winget install --id Kitware.CMake --exact
```

Installa **[Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/downloads/)** (sezione *Tools for Visual Studio*) oppure Visual Studio con il workload **Desktop development with C++ / Sviluppo di applicazioni desktop con C++**. Nell'installer assicurati di includere il compilatore **MSVC C++ x64**, un **Windows SDK** e gli strumenti C++ necessari. Non è sufficiente avere soltanto Visual Studio Code: serve una vera toolchain C++.

**Chiudi PowerShell e apri dal menu Start _Developer PowerShell for VS 2022_**. Da ora in poi usa quel terminale per NatLang. Controlla:

```powershell
git --version
cmake --version
cl
```

`cl` senza argomenti può stampare informazioni del compilatore e l'errore «no source files specified»: va bene, purché il comando sia riconosciuto. Se `cl` non esiste, riapri **Developer PowerShell** o aggiungi il workload C++ nell'installer.

**Download senza Git:** apri [NatLang su GitHub](https://github.com/gabrieleviolait/natlang), seleziona **Code → Download ZIP**, estrai la cartella e aprila dal terminale (`cd "PERCORSO_DELLA_CARTELLA"`). In questo caso salta il comando `git clone` del punto seguente.

### 3. Scaricare NatLang e posizionarsi nella cartella corretta

In **Developer PowerShell**, esegui:

```powershell
cd $HOME
git clone https://github.com/gabrieleviolait/natlang.git
cd .\natlang
Get-ChildItem
```

Devi vedere, tra gli altri, `CMakeLists.txt`, `src`, `examples`, `scripts` e `docs`. Se hai scaricato lo ZIP, esegui `cd` nella cartella **che contiene `CMakeLists.txt`**, non in `src`.

Per aggiornare una copia già clonata in seguito:

```powershell
git pull --ff-only
```

### 4. Compilare NatLang da sorgente (Windows)

Restando nella cartella `natlang`:

```powershell
cmake -S . -B build
cmake --build build --config Release --parallel
```

Con il generatore Visual Studio/MSVC, il compilatore risulta normalmente qui:

```powershell
.\build\Release\natc.exe --help
```

Dovresti vedere `NatLang compiler v0.4 (C++20)` e i comandi disponibili. Se hai scelto **Ninja o MinGW**, l'eseguibile potrebbe essere invece `build\natc.exe`: verifica con `Get-ChildItem .\build -Recurse -Filter natc.exe` e usa quel percorso negli esempi successivi.

Se modifichi `src` o aggiorni il repository, ricompila con `cmake --build build --config Release --parallel`.

### 5. Prima prova: calcolatrice senza creare file

Questo comando **compila temporaneamente un programma nativo, lo esegue e mostra il risultato**:

```powershell
.\build\Release\natc.exe --eval "2 + 2"
.\build\Release\natc.exe --eval "2 plus 2"
.\build\Release\natc.exe --eval "2 più 2"
.\build\Release\natc.exe --eval "2 elevato a 5"
```

Risultati attesi, nell'ordine: `4`, `4`, `4`, `32`. Per una compilazione di sviluppo più rapida aggiungi `--opt-level 0` (es. `--eval "2 + 2" --opt-level 0`). **Non occorre avviare alcun server AI.**

### 6. Creare il primo programma `.nat` e ottenere un EXE

Puoi partire da un esempio già incluso, evitando errori di sintassi:

```powershell
.\build\Release\natc.exe .\examples\hello.nat --check
.\build\Release\natc.exe .\examples\hello.nat -o .\hello.exe --compiler cl
.\hello.exe
```

Per scrivere il tuo programma, nella cartella principale apri Blocco note:

```powershell
notepad .\mio_programma.nat
```

Inserisci il testo seguente e salva il file **come testo UTF-8 con estensione `.nat`** (non `.nat.txt`):

```text
Mostra "Ciao, NatLang!"
Chiedi all'utente un numero e salva in x
Se x è maggiore di 10
    Mostra "Il numero è maggiore di 10"
Altrimenti
    Mostra "Il numero è 10 o meno"
Fine
Mostra x * 2
```

Quindi, dal terminale:

```powershell
.\build\Release\natc.exe .\mio_programma.nat --check --explain
.\build\Release\natc.exe .\mio_programma.nat -o .\mio_programma.exe --compiler cl
.\mio_programma.exe
```

Scrivi un numero quando appare la richiesta. L'eseguibile **non richiede NatLang né il modello GGUF per girare**; possono comunque essere richieste le librerie di sistema/runtime della toolchain C++ usata per compilarlo.

**Nota sulla sintassi:** nella v0.4.0 un blocco `Se`, `Ripeti`, `Mentre` o `Funzione` deve essere chiuso da `Fine` oppure `End`. Le frasi totalmente arbitrarie non sono ancora comprese senza AI, e nemmeno l'AI garantisce un'interpretazione corretta.

### 7. Strumenti utili del compilatore

```powershell
# Controlla il programma senza creare un eseguibile
.\build\Release\natc.exe .\examples\bilingual_program.nat --check --explain

# Mostra sul terminale la rappresentazione intermedia comune (IR)
.\build\Release\natc.exe .\examples\bilingual_program.nat --emit-ir

# Salva il C++20 generato senza compilare un EXE
.\build\Release\natc.exe .\examples\bilingual_program.nat --emit-cpp .\generated.cpp

# Compila più velocemente durante gli esperimenti
.\build\Release\natc.exe .\examples\multilingual_math.nat -o .\math.exe --compiler cl --opt-level 0
.\math.exe
```

Riferimenti: [linguaggio](docs/LANGUAGE_REFERENCE.md), [matematica](docs/MATH.md), [multilingua](docs/MULTILINGUAL.md), [IR](docs/SEMANTIC_IR.md).

### 8. Facoltativo: installare `llama.cpp` e avviare un piccolo modello GGUF

Questo passaggio è necessario **solo** per usare `--llm`, `--llm-all` o `--llm-preview`. Il modello è eseguito **localmente** durante la traduzione, non dentro il programma EXE risultante. Prima di usarlo per file o rete, controlla sempre che il programma interpretato faccia davvero ciò che volevi.

In una normale finestra PowerShell:

```powershell
winget install llama.cpp
```

Riapri il terminale e verifica:

```powershell
llama-server --help
curl.exe --version
```

Se `llama-server` non è disponibile, controlla il `PATH` e consulta l'[installazione ufficiale di llama.cpp](https://github.com/ggml-org/llama.cpp/blob/master/docs/install.md). Puoi usare anche una versione precompilata scaricata dalle [release ufficiali](https://github.com/ggml-org/llama.cpp/releases). Se `curl.exe` manca, installalo separatamente prima di usare le funzioni LLM.

**Terminale 1 — server AI (da lasciare aperto):** torna alla cartella `natlang` e avvia:

```powershell
.\scripts\start_gguf.ps1 -Model qwen3
```

Lo script usa `llama-server` su `127.0.0.1:8080`, con un modello **Qwen3-0.6B Q4_K_M** disponibile su Hugging Face; al primo avvio i pesi possono essere scaricati automaticamente e occorre attendere che il server li abbia caricati. Nessun modello è incluso nel repository NatLang. Il download richiede connessione Internet, mentre l'inferenza successiva può avvenire offline se i pesi sono disponibili nella cache.

Se PowerShell blocca lo script per la execution policy, **senza modificare permanentemente le impostazioni di sistema** puoi avviarlo così:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\start_gguf.ps1 -Model qwen3
```

**Terminale 2 — usa NatLang (sempre dalla cartella `natlang`):**

```powershell
# Anteprima di ciò che ha capito il modello: non esegue programmi
.\build\Release\natc.exe .\examples\gguf_free_form_italiano.nat --llm-preview

# Compilazione dopo aver esaminato l'anteprima
.\build\Release\natc.exe .\examples\gguf_free_form_italiano.nat --llm-all --show-normalized -o .\ai_demo.exe --compiler cl
.\ai_demo.exe
```

La differenza tra le modalità è importante:

| Flag | Comportamento |
| --- | --- |
| Nessun flag LLM | Solo parser deterministico, nessun modello richiesto |
| `--llm` | Chiama il modello **solo se** il parser fallisce |
| `--llm-all` | Passa **sempre** dal modello, anche su programmi già validi |
| `--llm-preview` | Normalizza e mostra programma + IR **senza generare o eseguire un EXE** |
| `--show-normalized` | Visualizza la traduzione generata quando differisce dalla sorgente |
| `--llm-profile qwen3` | Profilo del prompt per modelli piccoli (predefinito) |

**GGUF già sul disco:** usa `-GGUFPath "C:\percorso\modello.gguf"` con `start_gguf.ps1` al posto di `-Model qwen3`. Per una porta diversa aggiungi `-Port 8081` e indica a `natc` `--llm-url http://127.0.0.1:8081/v1/chat/completions`.

**Limite reale:** l'integrazione del protocollo è testata con un server simulato; non sono ancora confermati accuratezza o prestazioni effettive di Qwen3 sui 59 casi di benchmark. Il modello non è stato addestrato appositamente per NatLang e può produrre interpretazioni errate anche quando compilabili.

### 9. Facoltativo: Python, test e benchmark del modello

Installa **Python 3** da [python.org](https://www.python.org/downloads/) (o dal package manager), riapri il terminale e verifica `python --version`. Dalla cartella `natlang`:

```powershell
# Test automatici (il compilatore deve già essere stato costruito)
$env:NATC = (Resolve-Path .\build\Release\natc.exe).Path
python -m unittest discover -s tests -v

# Benchmark senza AI: riferimento deterministico
python .\benchmarks\evaluate.py --natc .\build\Release\natc.exe --mode deterministic --report baseline.json

# Benchmark con Qwen3 (richiede server del punto 8, attivo nel terminale 1)
python .\benchmarks\evaluate.py --natc .\build\Release\natc.exe --mode gguf --model-label Qwen3-0.6B-Q4_K_M --report qwen3-results.json
```

Il benchmark confronta l'IR con esempi di riferimento: non dimostra da solo che due programmi abbiano esattamente lo stesso comportamento. [Dettagli e limiti della valutazione](docs/GGUF_EVALUATION.md).

### 10. Linux e macOS: percorso essenziale da zero

**Ubuntu / Debian:** installa la toolchain e scarica il codice:

```bash
sudo apt update
sudo apt install -y git cmake g++ curl python3
git clone https://github.com/gabrieleviolait/natlang.git
cd natlang
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/natc --help
./build/natc --eval '2 plus 2' --opt-level 0
./build/natc examples/hello.nat -o hello --compiler g++ --opt-level 0
./hello
```

**macOS:** installa gli strumenti Apple (`xcode-select --install`), poi [Homebrew](https://brew.sh/) se non è già presente. Con Homebrew installa `cmake` e `git` (`brew install cmake git`) e usa gli stessi comandi Git/CMake del blocco Linux. `natc` usa Clang se disponibile; puoi selezionarlo esplicitamente con `--compiler clang++`. L'eseguibile risulterà normalmente in `./build/natc`.

Per l'AI su macOS, **`brew install llama.cpp`**; per Linux consulta le [modalità di installazione di llama.cpp](https://github.com/ggml-org/llama.cpp/blob/master/docs/install.md). Dopo aver installato `llama-server`, dalla cartella `natlang`:

```bash
bash ./scripts/start_gguf.sh qwen3
# In un secondo terminale, nella cartella natlang:
./build/natc examples/gguf_free_form_italiano.nat --llm-preview
```

### 11. Risoluzione dei problemi più frequenti

| Problema | Come risolvere |
| --- | --- |
| `cmake` o `git` non riconosciuto | Chiudi/riapri il terminale; verifica installazione e `PATH` |
| `cl` non riconosciuto | Apri **Developer PowerShell for VS 2022**, verifica il workload **Desktop development with C++** |
| `CMakeLists.txt` non trovato | Fai `cd` nella cartella principale del repository |
| `natc.exe` non trovato | Compila prima; verifica se è in `build\Release\natc.exe` oppure `build\natc.exe` |
| `C++ compilation failed` | Il compilatore C++ deve essere richiamabile anche quando `natc` compila il `.nat`; riprova con `--compiler cl` nel Developer PowerShell |
| `Unknown variable`, errore alla riga N, `Missing END` | Controlla nomi, virgolette e blocchi `Fine`/`End`; usa `--check --explain` |
| `llama-server` non trovato | Installa `llama.cpp`, riapri PowerShell e verifica il `PATH` |
| `Local llama-server request failed` | Verifica che il **primo terminale** sia ancora aperto, che il modello sia caricato, `curl` sia installato e porta/URL coincidano |
| Traduzione AI sbagliata ma compilabile | Usa `--llm-preview` e correggi il testo: il modello **non** garantisce equivalenza semantica |
| `Scan network` non vede dispositivi | Il ping ICMP può essere filtrato da firewall/rete: assenza di risposta **non** significa host spento |

**Sicurezza:** non eseguire programmi ricevuti da sconosciuti con privilegi elevati. NatLang non è una sandbox. Usa `Scan network` e `Scan IP` soltanto su sistemi autorizzati. Con un LLM locale, verifica sempre l'anteprima prima di compilare operazioni su file o rete.

**Link di partenza:** [repository](https://github.com/gabrieleviolait/natlang) · [esempi `.nat`](examples) · [linguaggio](docs/LANGUAGE_REFERENCE.md) · [integrazione GGUF](docs/GGUF_EVALUATION.md) · [problemi/segnalazioni](https://github.com/gabrieleviolait/natlang/issues).

---

## English quick start

For the **full zero-to-first-program walkthrough**, see the numbered Italian guide above; the commands work as shown even if you do not speak Italian. On Windows, install Git, CMake and Visual Studio 2022 Build Tools with the **Desktop development with C++** workload; open **Developer PowerShell for VS 2022**:

```powershell
git clone https://github.com/gabrieleviolait/natlang.git
cd natlang
cmake -S . -B build
cmake --build build --config Release
.\build\Release\natc.exe --eval "2 plus 2"
.\build\Release\natc.exe .\examples\hello.nat -o .\hello.exe --compiler cl
.\hello.exe
```

The LLM is **optional**. On Windows, install `llama.cpp` and run `.\scripts\start_gguf.ps1 -Model qwen3` in one PowerShell terminal; in another, from the repository root, run `.\build\Release\natc.exe .\examples\gguf_free_form_italiano.nat --llm-preview`. For Linux/macOS, see section 10 above. The rest of this README provides syntax, architecture and reference details in English.

---

## NEW: NatLang Studio Easy (Windows x64) — graphical app + one-click installer

Want to try NatLang without Git, CMake, or Visual Studio? Choose the **Windows Easy package**. It contains `natlang-studio.exe` (graphical editor), `natc.exe` (native compiler), a self-contained C++ toolchain (llvm-mingw), a local `llama.cpp` CPU runtime, documentation and examples. No programming setup is required after installing it.

**[Open the Easy Windows build workflow](https://github.com/gabrieleviolait/natlang/actions/workflows/easy-windows.yml)** to download the artifact **from a successful run** (GitHub sign-in), or use [Releases](https://github.com/gabrieleviolait/natlang/releases) if an installer has been published. The build produces both a **single setup `.exe`** and a **portable `.zip`**. Build assets are generated by GitHub Actions; the source repository does not store large third-party binaries.

1. Run `NatLang-Studio-Easy-Setup.exe`, or extract the ZIP and double-click `natlang-studio.exe`.
2. In the editor write `Mostra 2 più 2` and click **Esegui** → expected output: `4`.
3. Click **Salva** for `.nat` sources, **Verifica** for syntax, **Compila** for a native `.exe`, or **Anteprima AI** to review a natural-language translation. Interactive program input goes in the bottom field. **Stop** cancels a running compile/program.
4. To enable local AI click **Avvia AI** and keep its PowerShell window open. The first launch of a standard bundle fetches the ~484 MB Qwen3 GGUF model (internet required). The optional Full packaging mode includes the model for fully offline AI. Programs compiled without AI can already run offline.

Read the complete [**Easy installation & usage guide**](dist/EASY_START.md). Windows Easy installer/bundle builds are configured in [GitHub Actions](.github/workflows/easy-windows.yml); **a workflow succeeding has to be checked before claiming the installer is verified on Windows**. Standard source builds remain fully supported below.

## Try it

```text
2 + 2
2 plus 2
2 più 2
Mostra 2 elevato a 5
Mostra radice quadrata di 81
Mostra 15 per cento di 200
Chiedi all'utente un numero e salva in x
Se x è maggiore di 10
    Mostra "Grande"
Altrimenti
    Mostra "Piccolo"
Fine
```

The first six expressions print `4`, `4`, `4`, `32`, `9`, `30`. Later statements prompt and branch according to the input.

```text
Imposta n a 3
Repeat 2 times
    Increase n by 1
Fine
Show n
```

This compiles to a native executable and prints `5`. Italian and English statements share the same internal statement IR and backend.

## Build the compiler

Requires **CMake ≥ 3.16** and a **C++20 compiler**. `natc` itself is written in C++20 and needs neither Python nor an LLM to run. A host C++ toolchain is required to compile `.nat` into native executables.

**Windows (Developer PowerShell with Visual Studio Build Tools / MSVC):**

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\natc.exe examples\multilingual_math.nat -o math.exe --compiler cl
.\math.exe
```

**Linux / macOS (Clang or GCC):**

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/natc examples/multilingual_math.nat -o math
./math
```

Single expression:

```sh
./build/natc --eval "2 plus 2"   # 4
./build/natc --eval "2 ^ 8"      # 256
```

Inspect / compile:

```sh
./build/natc examples/bilingual_program.nat --check --explain
./build/natc examples/bilingual_program.nat --emit-ir program.ir.json
./build/natc examples/bilingual_program.nat --emit-cpp program.cpp
./build/natc examples/bilingual_program.nat -o program --keep-cpp
```

## Model-assisted free-form language (v0.4)

NatLang now includes a small-model prompt profile (`qwen3`), schema-constrained structured responses, one bounded validation-repair retry and a **preview mode that never executes generated code**. You need a separately installed local `llama-server` and a GGUF of your choice.

```powershell
# Windows: first terminal, after winget install llama.cpp
.\scripts\start_gguf.ps1 -Model qwen3
# second terminal (after building natc)
.\build\Release\natc.exe examples\gguf_free_form_italiano.nat --llm-preview
python benchmarks\evaluate.py --natc .\build\Release\natc.exe --mode gguf --model-label Qwen3-0.6B-Q4_K_M --report qwen3-results.json
```

`--llm` only calls the model when deterministic parsing fails; `--llm-all` always normalizes; `--llm-preview` always normalizes and shows canonical source plus IR (no execution). Review the results: semantic interpretation is **not** guaranteed. [GGUF setup, benchmark and limitations](docs/GGUF_EVALUATION.md).

## Supported language constructs

| Capability | Italian | English |
| --- | --- | --- |
| Assignment | `Imposta x a 5` | `Set x to 5` |
| Printing | `Mostra x * 2` | `Show x * 2` |
| Input | `Chiedi all'utente un numero e salva in x` | `Ask user for a number and store in x` |
| Conditional | `Se x è maggiore di 3` | `If x is greater than 3` |
| Alternative | `Altrimenti` | `Otherwise` |
| Counted loop | `Ripeti 3 volte` | `Repeat 3 times` |
| Condition loop | `Ripeti finché x è uguale a 3` | `Repeat until x equals 3` |
| While loop | `Mentre x < 5` | `While x < 5` |
| Function | `Funzione doppio(x)` | `Function double(x)` |
| Return | `Restituisci x * 2` | `Return x * 2` |
| List | `Crea una lista chiamata numeri` | `Create a list named numbers` |
| Append | `Aggiungi 4 a numeri` | `Add 4 to numbers` |
| End block | `Fine` | `End` |
| Local ICMP | `Scansiona la rete locale` | `Scan network` |

Quoted strings retain their original content. Blocks **currently require `Fine` or `End`**; indentation does not define blocks. Identifier names are currently ASCII, and case-sensitive.

See [GGUF Evaluation](docs/GGUF_EVALUATION.md), [Language Reference](docs/LANGUAGE_REFERENCE.md), [Multilingual guide](docs/MULTILINGUAL.md) and [Examples](examples). Spanish / French / German keyword support is **partial and experimental**, not at the same deterministic coverage level as IT/EN. The local model can attempt freer variants in all five languages.

## Mathematics

Native execution supports arithmetic `+ - * / % ^ **`, parentheses, unary signs, decimal values, and right-associative exponents; e.g. `2 ^ 3 ^ 2` is `512`, while `-2 ^ 2` is `-4`.

| Concept | Expression examples |
| --- | --- |
| Addition/subtraction | `2 plus 2`, `2 più 2`, `10 meno 3` |
| Multiplication/division | `5 per 4`, `7 diviso 2`, `3 times 4` |
| Remainder | `10 modulo 3` |
| Powers | `2 elevato a 5`, `pow(2,5)`, `potenza(2,5)` |
| Roots | `sqrt(81)`, `radice(81)`, `radice quadrata di 81`, `cbrt(27)` |
| Percentages | `15 per cento di 200`, `percent(15,200)`, `percentuale(15,200)` |
| More math | `abs`, `round`, `floor`, `ceil`, `ln`, `log10`, `exp`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `factorial`, `clamp`, `sign` |
| Lists | `somma([2,4,6])`, `media([2,4,6])`, `minimo([2,4,6])`, `massimo([2,4,6])` |
| Constants | `pi`, `euler` |

**Radians** are used for trigonometry; `log()` means natural logarithm. Invalid operations such as division by zero, negative square root and invalid logarithm domain raise runtime errors. All numbers currently use IEEE-754 `double`, not arbitrary precision or decimal financial arithmetic. [Full mathematical reference](docs/MATH.md).

## Less rigid syntax via a local LLM

The deterministic parser is the fast, inspectable default. With `--llm`, NatLang asks a local `llama.cpp` inference server **only if deterministic parsing fails**. With `--llm-all`, it tries to normalize every program, including prose statements that may be syntactically valid but meant differently. The model produces **canonical NatLang source**, then the same compiler parses, validates and emits C++; the executable does **not** depend on AI.

Start a compatible local model yourself:

```sh
llama-server -m /path/to/instruction-model.gguf --host 127.0.0.1 --port 8080 -c 4096
./build/natc examples/free_form_multilingual.nat --llm-all --show-normalized --keep-cpp -o demo
```

The model is **not bundled or benchmarked**. `llama.cpp` must be installed separately, and `curl` must be available. The endpoint is restricted to loopback. Always review the normalized program, especially before running code that accesses local files or networks. See [Local LLM](docs/LOCAL_LLM.md) and [Natural-language design](docs/NATURAL_LANGUAGE.md).

## Project documentation

- [Idea in italiano](docs/IDEA_IT.md) · [Vision](docs/VISION.md)
- [Architecture and shared IR](docs/ARCHITECTURE.md) · [IR details](docs/SEMANTIC_IR.md)
- [Language Reference](docs/LANGUAGE_REFERENCE.md) · [Multilingual](docs/MULTILINGUAL.md) · [Math](docs/MATH.md)
- [LLM integration](docs/LOCAL_LLM.md) · [Natural-language roadmap](docs/NATURAL_LANGUAGE.md) · [Roadmap](docs/ROADMAP.md)
- [Changelog](CHANGELOG.md) · [Contributing](CONTRIBUTING.md) · [Security](SECURITY.md)

## Tests

```sh
python3 -m unittest discover -s tests -v
```

The tests exercise native compilation and execution, IT/EN + selected ES/FR/DE programs, mathematical behavior and errors, input/output, lists, file operations and a **mock** `llama-server`. Mock protocol tests **do not demonstrate the accuracy of a real LLM**. GitHub Actions runs Linux and Windows jobs; platform-specific results must be checked on GitHub.

## Limitations and responsible use

- Arbitrary free-form prose, modules, classes, imports, GUI, robust general networking and complete static typing are **not** implemented. The statement IR is not yet a fully typed expression AST.
- Model translation can be plausible yet wrong. Never assume compiling proves semantic equivalence to the original natural-language request.
- `Scan network` is a bounded, ICMP-only scan of private LAN interfaces; an ICMP timeout does not prove a host is offline. Scan only networks and hosts you are authorized to inspect.
- Compiler paths and the chosen toolchain are trusted inputs; this prototype is **not** a secure sandbox for hostile source programs.

**License:** MIT — © 2026 Gabriele Viola and NatLang contributors. Contributions welcome.

**Build-speed tip:** use `--opt-level 0` for faster generated-program compilation during development; the default remains `--opt-level 2`. This only affects the host C++ compilation step.
