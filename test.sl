; solo lisp test
(def asd (require "asd\n\uA789'"))

(/ (+ 2 (* 7 8)) 9)

(|| 1 2 3 4)

(def x (if (=== 4 5) "yes" "no"))

(def obj {
  a
  (function (a) (a))
  v
  3
  c
  6
  })

(def square (function (x)
  (def list [3 1 2])
  (* x x)))

(console.log (new Date))

(function? 2)

(null? null)
(true? false)

(instanceof "asd" Date)
(void 0)
(% (+ (- 5 1) 4) 2)
(set! obj.v 6)
