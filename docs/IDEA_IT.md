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

È disponibile un **MVP sperimentale funzionante**, non un linguaggio universale privo di sintassi. Implementa variabili dinamiche, condizioni, cicli, funzioni, liste, input/output e file. Non include ancora GUI, networking, librerie esterne, classi o un LLM addestrato per NatLang.

Per le istruzioni operative consultare il [README](../README.md), per ciò che il compilatore comprende davvero la [Language Reference](LANGUAGE_REFERENCE.md), per l'architettura [Architecture](ARCHITECTURE.md) e per la direzione futura [Vision](VISION.md).
