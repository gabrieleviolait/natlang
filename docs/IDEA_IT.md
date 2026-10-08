# NatLang — idea progettuale (italiano)

**Obiettivo:** realizzare un linguaggio di programmazione general-purpose il cui codice sorgente sia composto da istruzioni vicine al linguaggio naturale, traducibili in **programmi nativi** compilati in C/C++. Un piccolo LLM locale potrebbe interpretare formulazioni libere, ma **non deve essere necessario per eseguire il programma compilato**.

## Problema

La programmazione tradizionale richiede sintassi esatta; gli assistenti AI trasformano prompt in codice ma non sono, di per sé, un linguaggio formalizzato. NatLang vuole introdurre uno strato semantico verificabile fra la descrizione umana e il codice macchina.

## Strategia

- Implementazione del compilatore in **C++20**.
- Parser deterministico per istruzioni comuni e un **LLM locale opzionale** per normalizzare frasi meno standard.
- Rappresentazione strutturata (AST; evoluzione prevista in IR tipizzato), controlli semantici e generazione di C++.
- Clang/GCC/MSVC come backend di compilazione, almeno nella prima fase.
- Eseguibili indipendenti dal modello AI e dal compilatore NatLang.
- Modalità di ispezione delle istruzioni interpretate e del C++ generato.

## Esempi

```text
Set starting to 5
Ask the user for a number and store it in answer
Repeat until answer equals 10
    Ask for a number and store it in answer
End
Show starting
```

Questo esempio è **supportato dalla versione v0.1**. Una frase interamente libera come «Prendi 5, continua a chiedere un numero finché l'utente scrive 10 e poi mostralo» è invece un **obiettivo**: oggi richiede la normalizzazione opzionale con un modello locale, e l'accuratezza non è garantita.

## Perché il progetto è interessante

La caratteristica distintiva da perseguire è l'insieme di queste proprietà: leggerezza, uso offline, indipendenza del binario dall'LLM, comprensione di formulazioni alternative, semantica verificabile e possibilità di sviluppare applicazioni generiche tramite librerie e moduli.

## Stato attuale

È disponibile un **MVP sperimentale funzionante**, non un linguaggio universale privo di sintassi. Implementa variabili dinamiche, condizioni, cicli, funzioni, liste, input/output, file e una prima funzione di discovery LAN via ping. Non include ancora GUI, networking generale, librerie esterne, classi o un LLM addestrato per NatLang.

La v0.2 amplia il linguaggio deterministico:

```text
2 + 2
2 plus 2
Chiedi all'utente un numero e salva in eta
Mostra eta + 1
Scan IP 192.168.1.1
Scan network
```

`natc --eval "2 plus 2"` produce direttamente `4`. La scansione IP usa ping senza privilegi speciali richiesti dal programma, ma il sistema operativo può limitarlo; l'assenza di risposta ICMP non significa necessariamente host spento. `Scan network` si limita ad alcune sottoreti LAN IPv4 private, con un limite esplicito al numero di host.

Per le istruzioni operative consultare il [README](../README.md), per ciò che il compilatore comprende davvero la [Language Reference](LANGUAGE_REFERENCE.md), per l'architettura [Architecture](ARCHITECTURE.md) e per la direzione futura [Vision](VISION.md).


## Aggiornamento v0.3.0

Il compilatore comprende ora un sottoinsieme molto più ampio di istruzioni italiane e inglesi, espressioni matematiche e costrutti di controllo. Sono presenti forme base sperimentali in spagnolo, francese e tedesco, con un'unica rappresentazione intermedia (`--emit-ir`) e un unico backend C++20. Un LLM locale opzionale può normalizzare istruzioni più libere mediante `--llm` o `--llm-all`; il modello **non è incluso**. Questa release non fornisce ancora la comprensione universale del linguaggio naturale né un sistema semantico completamente tipizzato.
