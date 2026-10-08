# Math library and numeric semantics (NatLang v0.3)

## Operators

| Precedence high to low | Operators | Meaning |
| --- | --- | --- |
| 7 | `^`, `**` | Power, right-associative |
| 7 | unary `+` / `-` | Positive/negative sign; `-2^2` = `-4` |
| 5 | `*`, `/`, `%` | Multiply, floating division, floating remainder (`fmod`) |
| 4 | `+`, `-` | Addition, subtraction; `+` concatenates if an operand is text |
| 3 | `==`, `!=`, `<`, `>`, `<=`, `>=` | Comparison |
| 2 | `and`, `e` | Boolean conjunction |
| 1 | `or`, `o` | Boolean disjunction |

`( ... )` overrides precedence. Literal numbers are decimal; the program runtime represents numbers as IEEE-754 `double`, so precision is limited. Exponents such as `2^3^2` evaluate as `2^(3^2)`.

## Functions

| Function | Italian alias | Arity / behavior |
| --- | --- | --- |
| `pow(a,b)` | `potenza(a,b)` | Exponentiation |
| `sqrt(x)` | `radice(x)` | Square root; x ≥ 0 |
| `cbrt(x)` | `radice_cubica(x)` | Cube root |
| `abs(x)` | `assoluto(x)` | Absolute value |
| `round(x)` | `arrotonda(x)` | Round halfway away from zero |
| `floor(x)` | `arrotonda_giu(x)` | Round down |
| `ceil(x)` | `arrotonda_su(x)` | Round up |
| `ln(x)` or `log(x)` | — | Natural logarithm; x > 0 |
| `log10(x)` | — | Base-10 logarithm; x > 0 |
| `exp(x)` | — | Natural exponential |
| `sin(x)`, `cos(x)`, `tan(x)` | `seno`, `coseno`, `tangente` | Angle in **radians** |
| `asin(x)`, `acos(x)`, `atan(x)`, `atan2(y,x)` | — | Result in **radians** |
| `factorial(x)` | `fattoriale(x)` | Integer 0–170 |
| `sign(x)` | `segno(x)` | −1, 0, +1 |
| `percent(rate,amount)` | `percentuale(rate,amount)` | `rate * amount / 100` |
| `clamp(x,min,max)` | `limita(x,min,max)` | Clip to interval; min ≤ max |
| `sum(list)` | `somma(list)` | Numeric total |
| `average(list)` | `media(list)` | Numeric mean, nonempty |
| `min(list)` / `max(list)` | `minimo` / `massimo` | List minimum / maximum |
| `length(value)` | `lunghezza(value)` | List or string length |

Constants: `pi` and `euler`. No implicit conversion from text to number: prompt for a number explicitly. Non-finite domain or overflow results for added math functions produce a runtime error. Floating arithmetic can lose precision; **do not use for exact decimal financial calculations**.

### Natural forms supported without AI

```text
2 plus 2
2 più 2
5 per 4
7 diviso 2
10 modulo 3
2 elevato a 5
3 al quadrato
radice quadrata di 81
15 per cento di 200
Mostra fattoriale(5)
```

The expression parser is not a full NLP reasoner; ask the optional local LLM to normalize nonstandard paraphrases.
