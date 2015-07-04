; solo lisp test
(def asd (require "escodegen"))

(/ (+ 2 (* 7 8)) 9)

(|| 1 2 3 4)

(def x (if (=== 4 5) "yes" "no"))

(def obj {
  a
  (function (a) (a))
  v
  3
  })

(def square (function (x)
  (def list [3 1 2])
  (* x x)))

(console.log (new Date))

(instanceof "asd" Date)
(void 0)
(set! obj.v 6)

(def count (function (from to)
  (if (>= from to)
    from
    (count (+ 1 from) to))))

(console.log (count 0 100000000))
