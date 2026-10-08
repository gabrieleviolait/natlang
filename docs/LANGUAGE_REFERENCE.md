# NatLang v0.2 language reference

This describes the **implemented**, deliberately restricted language. This is not a promise that arbitrary plain English will compile without the optional model.

## Source format

- Save text to a `.nat` file. One instruction per line.
- Blank lines, `# comment` and `// comment` lines are ignored.
- Indentation is visual; nested control blocks **must end with `End`**.
- Keywords and verbs match case-insensitively; identifiers are **case-sensitive**.
- Identifiers use ASCII letters, digits and underscore, starting with a letter/underscore. Reserved keywords cannot be used as variable names.
- Quoted text uses `'...'` or `"..."`. Literal text **must be quoted** to distinguish it from identifiers.
- Expression operators have typical arithmetic/logical precedence. Parentheses are supported.
- A **bare expression** is automatically evaluated and printed (`2 + 2`, `2 plus 2`, `2 più 2`). This also works with `natc --eval "2 plus 2"` (compiles and executes a temporary native program).

## Variables & output

```text
Set age to 30
Let city be "Rome"
Remember tax as 0.22
Increase age by 1
Decrease age by 2
Show "Age: " + age
Print city
2 + 2
2 plus 2
2 piu 2
Mostra 2 più 2
```

Numbers currently use `double`, not precise decimal currency or arbitrary-size integer arithmetic. String `+` concatenates with other values. Other arithmetic requires numbers. The runtime has no variables scoped to `If`/loop blocks.

## Conditions

```text
Set score to 87
If score is greater than or equal to 60
    Show "Pass"
Otherwise
    Show "Fail"
End
```

Comparisons: `==`, `!=`, `>`, `<`, `>=`, `<=`, `is`, `equals`, `is greater than`, `is less than`, `is greater than or equal to`, `is less than or equal to`, `is not equal to`, and `is different from`. Boolean operators: `and`, `or`, `not`; Boolean literals `true`, `false`. Expression parsing supports parentheses.

## Loops & control flow

```text
Set counter to 3
Repeat until counter equals 0
    Show counter
    Decrease counter by 1
End

Repeat 2 times
    Show "Hello"
End

While counter < 3
    Increase counter by 1
End
```

`Break` and `Continue` are supported inside loops. Infinite loops are possible and aren't automatically detected. `Repeat N times` accepts a nonnegative whole numeric value up to an implementation-defined runtime cap (currently 100,000,000).

## Input

```text
Ask for text and store it in name
Ask the user for a number and store it in amount
Ask for 3 numbers and store them in measurements
Ask user
Ask user "Your name? " and store in name
Ask user for a number and store it in age
Chiedi all'utente
Chiedi all'utente un numero e salva in eta
Chiedi ad utente "Nome? " e salva in nome
```

`Ask user`/`Chiedi all'utente` defaults to variable `answer` (text input) and shows an input prompt; the numbered forms parse numeric input into the named variable (or `answer` if unnamed). For text with a custom prompt, provide a quoted message and `and store in name` or `e salva in nome`. Legacy `Ask for ... and store it in ...` remains supported without an automatic prompt.

Input is line-oriented from standard input. Invalid numeric input raises a runtime error. `Ask for N numbers` prompts `Number 1:`, `Number 2:`, etc.; current cap is 100,000 inputs.

## Lists & operations

```text
Create a list named values
Add 5 to values
Append 7 into values
Put 11 to values
Show values
Show length(values)
Show sum(values)
Show average(values)
Show min(values)
Show max(values)
Set other to [1, 2, 3]
```

List items may have mixed dynamic types. Arithmetic aggregate functions expect compatible numeric values. `average`, `min` and `max` error for empty lists. `length` accepts a list or a string. There is currently no list indexing, removal, sorting or maps/dictionaries.

## Functions

```text
Define a function called square with parameter x
    Return x * x
End
Show square(5)
```

Alternative declaration: `Function square(x)`. Parameters are comma-separated identifiers. Functions are top-level only. User functions can call other functions and themselves (recursion) and return a runtime `Value`. They do not capture variables from the main program.

## File operations

```text
Save "Hello" to file "greeting.txt"
Load file "greeting.txt" into message
Show message
```

File paths are evaluated expressions, relative to the **working directory of the generated executable**, not the `.nat` location. Saving overwrites the destination. File I/O is synchronous, with text rendered through the runtime. There is no file sandbox.

## Networking: bounded IPv4 ICMP discovery

```text
Scan IP 192.168.1.1
Scan this ip 127.0.0.1
Set target to "192.168.1.20"
Scan this ip target
Scan this ip
Scan network
Show ping("127.0.0.1")
```

- `Scan IP` and `Scan this ip` execute **one system ping** for a validated dotted-decimal IPv4 address. Without an IP, `Scan this ip` prompts at runtime. An expression or string variable can supply the target. `ping(address)` evaluates to a Boolean and can be used in conditions.
- `Scan network` enumerates active non-loopback **private IPv4** interfaces. It probes at most **two** local /24 slices (up to 508 usable host addresses), with at most 16 concurrent probes and approximately one second of ICMP timeout each. Wider routed subnets are intentionally **not** exhaustively scanned; public networks and loopback are excluded from automatic discovery.
- No port scanning, OS detection, service identification, exploits, or credentials are involved. Reachability means ICMP response only; absence of response is **not** proof a host is offline. A system `ping` command and OS support are required; some locked-down environments forbid ping.
- No background scans: probes run only when the compiled program executes. Use only on networks and hosts where you have authorization.

## Error behavior

- Invalid/unknown phrases: a line-numbered compilation error.
- Unknown variables/functions, wrong function arity and malformed expressions: compilation errors.
- Read of a known but unassigned variable, division by zero, invalid input types: runtime errors.
- Contradictory, underspecified or semantically different natural-language instructions may normalize incorrectly with a model; compilation success doesn't guarantee intended behavior.

## What is *not* supported in v0.2

Object-oriented classes, exceptions in `.nat`, modules/imports, general networking (beyond IPv4 ICMP discovery), threads as a language feature, GUI, screen/keyboard automation, external libraries, dictionaries, detailed type annotations, array indexing, floating-point financial accuracy guarantees, and literal full free-form English without local LLM normalization. See [roadmap](ROADMAP.md).
