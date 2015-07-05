(def x 1 y 2 z 98.3)
(def a (function (x . ys) ys))
(console.log (a 1 2 3 4))
