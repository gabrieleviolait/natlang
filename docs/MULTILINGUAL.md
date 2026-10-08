# Multilingual programming in NatLang v0.3

## One compiler, one IR, multiple languages

Every supported sentence is transformed into the same internal `Node` statement representation and expression checker. Translation changes **only the frontend**. There is a single code generator, and generated programs run without a language model.

### Italian and English (primary deterministic languages)

```text
Imposta contatore a 0
Ripeti 3 volte
    Incrementa contatore di 2
Fine
Mostra contatore
```

```text
Set counter to 0
Repeat 3 times
    Increase counter by 2
End
Show counter
```

Both compile using the same internal operations. You may mix recognized Italian and English statements in the same file. `Se/If`, `Altrimenti/Otherwise`, `Fine/End`, `Funzione/Function`, `Ritorna/Return`, `Chiedi/Ask`, `Mostra/Show` and core list/file constructs are recognized, within the documented grammar.

### Partial additional languages (experimental)

```text
Establece n como 2
Si n es mayor que 1
    Muestra "hola"
Fin

Definis m a 3
Si m est supérieur à 2
    Affiche "bonjour"
Fin

Setze k auf 4
Wenn k ist größer als 3
    Zeige "hallo"
Ende
```

Basic statements and operators in **Spanish, French and German** are recognized, but this is a small tested set, **not full language coverage**. Expressions and accents are normalized outside quoted strings. UTF-8 keywords are recognized in documented spellings; there is no full locale-aware Unicode parser. User variable names are ASCII in v0.3.

### Free-form instructions

For less predictable grammar, use `--llm` or `--llm-all` with your **own local model**, which is prompted to output canonical NatLang. The output is then validated by the same deterministic compiler. This route is model-dependent and not guaranteed correct, regardless of the language. Review generated normalization via `--show-normalized` and the statement IR via `--emit-ir`.

### Caveats

- Source blocks require explicit end markers (`Fine`/`End` etc.).
- Strings must be quoted, so text is distinguishable from variable names.
- Variable/function names are case-sensitive ASCII identifiers; keywords are mostly ASCII case-insensitive.
- A phrase such as `ripeti finché...` is parsed by explicit grammar, not understanding of arbitrary prose; model fallback handles more unusual variants.
- Italian `per` is interpreted as multiplication **inside expressions**; in other contexts use canonical statements or model normalization.
