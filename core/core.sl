;;;
;;;   Solo Standard Prelude
;;;


;;;;;;;;;
;;; Atoms
(def 'nil '())
(def 'true 1)
(def 'false 0)


;;;;;;;;;
;;; Functional Functions

; Function Definitions
(def 'defn (fn '(f b) '(
  def (head f) (fn (tail f) b)
)))

; Open new scope
(defn '(let b) '(
  ((fn '_ b) ())
))

; Unpack List to Function
(defn '(unpack f l) '(
  eval (join (list f) l)
))

; Unapply List to Function
(defn '(pack f xs...) '(f xs))

; Curried and Uncurried calling
(def 'curry unpack)
(def 'uncurry pack)

; Perform Several things in Sequence
(defn '(do l...) '(
  if (== l nil)
    '(nil)
    '(last l)
))


;;;;;;;;;
;;; Logical Functions

; Logical Functions
(defn '(not x)   '(- 1 x))
(defn '(or x y)  '(+ x y))
(defn '(and x y) '(* x y))


;;;;;;;;;
;;; Numeric Functions

; Minimum of Arguments
(defn '(min xs...) '(
  if (== (tail xs) nil) '(fst xs)
    '(do
      (put 'rest (unpack min (tail xs)))
      (put 'item (fst xs))
      (if (< item rest) 'item 'rest)
    )
))

; Maximum of Arguments
(defn '(max xs...) '(
  if (== (tail xs) nil) '(fst xs)
    '(do
      (put 'rest (unpack max (tail xs)))
      (put 'item (fst xs))
      (if (> item rest) 'item 'rest)
    )
))


;;;;;;;;;
;;; Conditional Functions

(defn '(select cs...) '(
  if (== cs nil)
    '(error "No Selection Found")
    '(if (fst (fst cs)) '(snd (fst cs)) '(unpack select (tail cs)))
))

(defn '(case x cs...) '(
  if (== cs nil)
    '(error "No Case Found")
    '(if (== x (fst (fst cs))) '(snd (fst cs)) '(
    unpack case (join (list x) (tail cs))))
))

(def 'otherwise true)


;;;;;;;;;
;;; Misc Functions

(defn '(flip f a b) '(f b a))
(defn '(comp f g x) '(f (g x)))


;;;;;;;;;
;;; List Functions

; First, Second, or Third Item in List
(defn '(fst l) '( eval (head l) ))
(defn '(snd l) '( eval (head (tail l)) ))
(defn '(trd l) '( eval (head (tail (tail l))) ))

; List Length
(defn '(len l) '(
  if (== l nil)
    '0
    '(+ 1 (len (tail l)))
))

; Nth item in List
(defn '(nth n l) '(
  if (== n 0)
    '(fst l)
    '(nth (- n 1) (tail l))
))
