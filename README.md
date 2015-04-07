# Solo

_A small Scheme aimed at developer happiness and productivity in web programming_

### =

Tests equality of two atoms.

### throw

Throws and error and stops execution.

### nil?

Returns `true` is given parameter is strictly equal to `nil`

### true?

Returns `true` is given parameter is strictly equal to `true`

### false?

Returns `true` is given parameter is strictly equal to `false`

### symbol

Converts a string to a symbol.

### symbol?

Returns `true` is given parameter is a symbol, `false` otherwise.

### keyword

Converts a string to a keyword.

### keyword?

Returns `true` is given parameter is a keyword, `false` otherwise.

---

### pr-str

Returns a pretty string representation of the value passed in

### str

Returns a string representation of the value passed in

### prn

Prints to stdout a pretty string representation of the value passed in

### println

Prints to stdout a string representation of the value passed in

### str-split

Splits a string on given delimiter returning a list.

```scheme
solo> (str-split "asd/asd/wq/d.scm" "/")
("asd" "asd" "wq" "d.scm")
```

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

### `+ - * / %`

All the previous functions take two integers and operates on them returning an other int
as expected

### time-ms

Returns current milliseconds elapsed count since epoch.

---

### list

Wraps elements passed in a list.

```scheme
solo> (list 1 :a "a")
(1 :a "a")
solo> (list 4 [5])
(4 [5])
```

### list?

Returns a `true` if the atom passed in is a list, `false` otherwise.

### vector

Wraps elements passed in a vector.

```scheme
solo> (vector 1 'a)
[1 a]
solo> (vector [1 'a])
[[1 a]]
```

### vector?

Returns a `true` if the atom passed in is a vector, `false` otherwise.

### hash-map

Wraps elements passed in a hashmap.

```scheme
solo> (hash-map "a" 1)
{"a" 1}
solo> (hash-map :a 1 :b 2)
{:a 1 :b 2}
```

### map?

Returns a `true` if the atom passed in is a map, `false` otherwise.

### assoc

Takes a hashmap and associates vars to values in it.

```scheme
solo> (assoc {:a 1} :b 2)
{:b 2 :a 1}
solo> (assoc {:a 1 :b 2} :a 100)
{:a 100 :b 2}
```

### dissoc

Deletes a key from a hashmap.

```scheme
solo> (dissoc {:a 1 :b 2} :a)
{:b 2}
```

### get

Gets the value at a given key from a hashmap.

### contains?

Return `true` if the given hashmap contains the specified key,
`false` otherwise.

### keys

Returns a list of all the keys in the given map.

### vals

Returns a list of all the values in the given map.

---

### sequential?

Returns `true` if the given parameter is a **list** or
a **vector**, `false` otherwise.

### cons

Prepends a given atom to a sequence, always returns a list.

```scheme
solo> (cons 4 '(1 2 3))
(4 1 2 3)
solo> (cons 4 [1 2 3])
(4 1 2 3)
```

### concat

Concatenates sequences together, always returns a list.

```scheme
solo> (concat [0] '(1 2) [3 4 5])
(0 1 2 3 4 5)
```

### empty?

Returns `true` if the given sequence is empty, `false` otherwise.

### nth

Returns the **nth** element of a sequence.

```scheme
solo> (nth [:g :h :c :i] 3)
:i
solo> (nth '(:g :h :c :i) 0)
:g
```

### first

Returns the first atom in a sequence.

### rest

Returns all but the first atom in a sequence.

### count

Returns the number of items contained in a sequence.

### apply

Takes a function and arguments to pass in, evaluates the given arguments and then, call the given function with them.

```scheme
solo> (apply + 3 (list 5))
8
```

### map

Takes a function and a list and applies each element in the list to the function returning the resulting list

```scheme
solo> (map (fn* [x] (* x x)) [1 2 3 4 5])
(1 4 9 16 25)
```

### conj

Prepends atom to a given list or appends atom to a given vector

```scheme
solo> (conj '(1 2 3) 4)
(4 1 2 3)
solo> (conj [1 2 3] 4)
[1 2 3 4]
```

---

### with-meta

Associates metadata to a function. Metadata must be a hashmap.

### meta

Retrives the metadata associated with a function.

### atom

Returns the atom you pass-in.

### atom?

Returns `true` if the given parameter is an atom.

### deref
### reset!
### swap!

Replaces an already **def!**ined symbol (pointing to an atom)
in the environment with the result of calling the provided
function with the symbols current value.

```scheme
solo> (def! a (atom 5))
(atom 5)
solo> (swap! a (fn* (a) (* 2 a)))
10
solo> a
(atom 10)
```

---

## Prelude in solo

### load-file

Reads, parses and evaluates a file's contents.

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
