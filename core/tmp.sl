












; Last item in List
(fun {last l} {nth (- (len l) 1) l})

; Apply Function to List
(fun {map f l} {
  if (== l nil)
    {nil}
    {join (list (f (fst l))) (map f (tail l))}
})

; Apply Filter to List
(fun {filter f l} {
  if (== l nil)
    {nil}
    {join (if (f (fst l)) {head l} {nil}) (filter f (tail l))}
})

; Return all of list but last element
(fun {init l} {
  if (== (tail l) nil)
    {nil}
    {join (head l) (init (tail l))}
})

; Reverse List
(fun {reverse l} {
  if (== l nil)
    {nil}
    {join (reverse (tail l)) (head l)}
})

; Fold Left
(fun {foldl f z l} {
  if (== l nil) 
    {z}
    {foldl f (f z (fst l)) (tail l)}
})

; Fold Right
(fun {foldr f z l} {
  if (== l nil) 
    {z}
    {f (fst l) (foldr f z (tail l))}
})

(fun {sum l} {foldl + 0 l})
(fun {product l} {foldl * 1 l})

; Take N items
(fun {take n l} {
  if (== n 0)
    {nil}
    {join (head l) (take (- n 1) (tail l))}
})

; Drop N items
(fun {drop n l} {
  if (== n 0)
    {l}
    {drop (- n 1) (tail l)}
})

; Split at N
(fun {split n l} {list (take n l) (drop n l)})

; Take While
(fun {take-while f l} {
  if (not (unpack f (head l)))
    {nil}
    {join (head l) (take-while f (tail l))}
})

; Drop While
(fun {drop-while f l} {
  if (not (unpack f (head l)))
    {l}
    {drop-while f (tail l)}
})

; Element of List
(fun {elem x l} {
  if (== l nil)
    {false}
    {if (== x (fst l)) {true} {elem x (tail l)}}
})

; Find element in list of pairs
(fun {lookup x l} {
  if (== l nil)
    {error "No Element Found"}
    {do
      (= {key} (fst (fst l)))
      (= {val} (snd (fst l)))
      (if (== key x) {val} {lookup x (tail l)})
    }
})

; Zip two lists together into a list of pairs
(fun {zip x y} {
  if (or (== x nil) (== y nil))
    {nil}
    {join (list (join (head x) (head y))) (zip (tail x) (tail y))}
})

; Unzip a list of pairs into two lists
(fun {unzip l} {
  if (== l nil)
    {{nil nil}}
    {do
      (= {x} (fst l))
      (= {xs} (unzip (tail l)))
      (list (join (head x) (fst xs)) (join (tail x) (snd xs)))
    }
})

;;; Other Fun

; Fibonacci
(fun {fib n} {
  select
    { (== n 0) 0 }
    { (== n 1) 1 }
    { otherwise (+ (fib (- n 1)) (fib (- n 2))) }
})
