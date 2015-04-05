# Solo

_A small Scheme aimed at developer happiness and productivity in web programming_

### =

Tests equality of two atoms

### throw

Throws and error and stops execution

### nil?
### true?
### false?
### symbol
### symbol?
### keyword
### keyword?

---

### pr-str

Returns a pretty string representation of the value passed in

### str

Returns a string representation of the value passed in

### prn

Prints to stdout a pretty string representation of the value passed in

### println

Prints to stdout a string representation of the value passed in

### readline

Readline from stdin, takes a str prompt as first parameter. Returns an error
if it fails to do so.

### read-string

Parses a string to solo actual datum.

```scheme
solo> (read-string "('x (+ 5 10))")
((quote x) (+ 5 10))
```

### slurp

Given a filename it return it's contents as a string

---

### `< <= > >=`

All the previous functions take two integers and compares them returning a bool
as expected

### `+ - * /`

All the previous functions take two integers and operates on them returning an other int
as expected

### time-ms

Returns current milliseconds

---

### list
### list?
### vector
### vector?
### hash-map
### map?
### assoc
### dissoc
### get
### contains?
### keys
### vals

---

### sequential?
### cons
### concat
### empty?
### nth
### first
### rest
### count
### apply
### map
### conj

---

### with-meta
### meta
### atom
### atom?
### deref
### reset!
### swap!

---

## Prelude in solo

### not
### load-file
### cond
### or

---

## Prelude in core.scm

### inc
### dec
### zero?
### reduce
### identity
### every?
### not
### some
### and
### or
### cond
### ->
### ->>
