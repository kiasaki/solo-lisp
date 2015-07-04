# Solo Lang

_Minimal lisp syntax to plain and readable JavaScript compiler_

**Solo** is not a separate language but really a way of writing JavaScript in a
_lispy_ syntax. See for yourself...

## Getting started

```
$ npm install -g solo
$ echo "(def square (function (x) (* x x))) (console.log (square 5))" > test.sl
$ solo test.sl
25
```

## Documentation

| Example | Solo | JavaScript |
|---|---|---|
| Operators | `(+ 1 2)` | `1 + 2;` |
| | `(/ 4 2)` | `4 / 2;` |
| | `(/ (* 2 6) 3)` | `(2 * 6) / 3;` |
| | `(% (+ (- 5 1) 4) 2)` | `(5 - 1 + 4) % 2;` |
| | `(=== 5 "5")` | `5 === "5";` |
| | `(instanceof "asd" Date)` | `"asd" instanceof Date;` |
| | `(typeof "rick")` | `typeof "rick";` |
| | `(void 0)` | `void 0;` |

## Supported Primitives

```
if function new
instanceof typeof void
+ - * / %
< <= > >= || && == === != !==
null? true? false?
undefined? boolean? number? string? object? array? function?
def set!
```

## Contribute

## License

**MIT** (see `LICENSE` file)
