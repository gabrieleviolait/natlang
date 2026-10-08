# NatLang v0.1 language reference

This describes the **implemented**, deliberately restricted language. This is not a promise that arbitrary plain English will compile without the optional model.

## Source format

- Save text to a `.nat` file. One instruction per line.
- Blank lines, `# comment` and `// comment` lines are ignored.
- Indentation is visual; nested control blocks **must end with `End`**.
- Keywords and verbs match case-insensitively; identifiers are **case-sensitive**.
- Identifiers use ASCII letters, digits and underscore, starting with a letter/underscore. Reserved keywords cannot be used as variable names.
- Quoted text uses `'...'` or `"..."`. Literal text **must be quoted** to distinguish it from identifiers.
- Expression operators have typical arithmetic/logical precedence. Parentheses are supported.

## Variables & output

```text
Set age to 30
Let city be "Rome"
Remember tax as 0.22
Increase age by 1
Decrease age by 2
Show "Age: " + age
Print city
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
```

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

## Error behavior

- Invalid/unknown phrases: a line-numbered compilation error.
- Unknown variables/functions, wrong function arity and malformed expressions: compilation errors.
- Read of a known but unassigned variable, division by zero, invalid input types: runtime errors.
- Contradictory, underspecified or semantically different natural-language instructions may normalize incorrectly with a model; compilation success doesn't guarantee intended behavior.

## What is *not* supported in v0.1

Object-oriented classes, exceptions in `.nat`, modules/imports, networking, threads, GUI, screen/keyboard automation, external libraries, dictionaries, detailed type annotations, array indexing, floating-point financial accuracy guarantees, and literal full free-form English without local LLM normalization. See [roadmap](ROADMAP.md).
