; solo lisp test
(def asd (require "escodegen"))

(/ (+ 2 (* 7 8)) 9)

(|| 1 2 3 4)

(def x (if (=== 4 5) "yes" "no"))

(def obj {
  a
  (function (a) (a))
  v
  3.5
  })

(def square (function (x)
  (def list [3 1 2])
  (* x x)))

(console.log (new Date))

(instanceof "asd" Date)
(void 0)
(set! obj.v 6)

; comment
(def count (function (from to)
  (if (>= from to)
    from
    (count (+ 1 from) to))))

(console.log (count 0 100))

(try
  (throw "work")
  (+ 1 2)
  (catch (e) (console.log e)))

(import
  (path)
  (ramda as R)
  (ramda refer (map)))

(get 0 [])
(get a {a 1})
(get v obj)
(get v (|| {} {}))

(do
  (def a null)
  (def b false)
  (def c 1))

(do)

(function ()
  (do))

