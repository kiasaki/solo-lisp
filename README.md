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

## Contribute

## License

**MIT** (see `LICENSE` file)
