# Solo Lang

[![Build Status](https://travis-ci.org/kiasaki/solo-lisp.svg)](https://travis-ci.org/kiasaki/solo-lisp)

_Minimal lisp syntax to plain and readable JavaScript compiler_

**Solo** is not a separate language but really a way of writing JavaScript in a
_lispy_ syntax. See for yourself...

## Getting started

```
$ npm install -g solo
$ echo "(def square (function (x) (* x x))) (console.log (square 5))" > test.sl
$ solo test.sl

'use strict';
let square = function (x) {
      return x * x;
};
console.log(square(5));

$ solo test.sl | node --harmony

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
| Conditionals | `(if (> 2 1) "yes" "no")` | `2 > 1 ? "yes" : "no";` |
| Assignment | `(def x 2)` | `let x = 2;` |
| | `(set! module.exports square)` | `module.exports = square;` |
| Objects | `(new Date)` | `new Date();` |
| | `(new Array 5)` | `new Array(5);` |
| | `(get 0 arr)` | `arr[0];` |
| | `(get "x" o)` | `o["x"];` |
| | `(get ch ba)` | `ba.ch;` |
| Types | `"str\nrts"` | `"str\nrts";` |
| | `4` | `4;` |
| | `4.5` | `4.5;` |
| | `true` | `true;` |
| | `false` | `false;` |
| | `null` | `null;` |
| | `[1 2 3]` | `[1, 2, 3];` |
| | `{a 1 b 2 c 3}` | `{a: 1, b: 2, c: 3};` |
| | `(fn prop1 prop2)` | `fn(prop1, prop2);` |
| | `; comment` | `` |
| Other | `(try)` | `try {} finally {};` |
| | `(try (catch (xx)))` | `try {} catch (xx) {}` |
| | `(try (throw "oops") (+ 1 2) (catch (e) (console.log e)))` | `try {throw 'oops'; 1 + 2;} catch (e) {console.log(e);}` |
| | `(do)` | `(function() {}());` |
| | `(do (set! a 1) (set! b "2"))` | `(function () {a = 1; b = "2";}())` |

## Supported Primitives

```
if function new try catch finally do
instanceof typeof void throw
def set!
+ - * / %
< <= > >= || && == === != !==
null? true? false?
undefined? boolean? number? string? object? array? function?
```

## Contribute

## License

**MIT** (see `LICENSE` file)
