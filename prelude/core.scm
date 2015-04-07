(def! inc (fn* (a) (+ a 1)))

(def! dec (fn* (a) (- a 1)))

(def! zero? (fn* (n) (= 0 n)))

(def! reduce
  (fn* (f init xs)
    (if (> (count xs) 0)
      (reduce f (f init (first xs)) (rest xs))
      init)))

(def! identity (fn* (x) x))

(def! every?
  (fn* (pred xs)
    (if (> (count xs) 0)
      (if (pred (first xs))
        (every? pred (rest xs))
        false)
      true)))

(def! not (fn* (x) (if x false true)))

(def! some
  (fn* (pred xs)
    (if (> (count xs) 0)
      (let* (res (pred (first xs)))
        (if (pred (first xs))
          res
          (some pred (rest xs))))
      nil)))

(defmacro! and
  (fn* (& xs)
    (if (empty? xs)
      true
      (if (= 1 (count xs))
        (first xs)
        `(let* (and_FIXME ~(first xs))
          (if and_FIXME (and ~@(rest xs)) and_FIXME))))))

(defmacro! or
  (fn* (& xs)
    (if (empty? xs)
      nil
      (if (= 1 (count xs))
        (first xs)
        `(let* (or_FIXME ~(first xs))
          (if or_FIXME or_FIXME (or ~@(rest xs))))))))

(defmacro! cond
  (fn* (& clauses)
    (if (> (count clauses) 0)
      (list 'if (first clauses)
            (if (> (count clauses) 1)
                (nth clauses 1)
                (throw "cond requires an even number of forms"))
            (cons 'cond (rest (rest clauses)))))))

(defmacro! ->
  (fn* (x & xs)
    (if (empty? xs)
      x
      (let* (form (first xs)
             more (rest xs))
        (if (empty? more)
          (if (list? form)
            `(~(first form) ~x ~@(rest form))
            (list form x))
          `(-> (-> ~x ~form) ~@more))))))

(defmacro! ->>
  (fn* (x & xs)
    (if (empty? xs)
      x
      (let* (form (first xs)
             more (rest xs))
        (if (empty? more)
          (if (list? form)
            `(~(first form) ~@(rest form) ~x)
            (list form x))
          `(->> (->> ~x ~form) ~@more))))))

(def! head first)
(def! tail rest)
(def! init (fn* (l)
  (reverse (tail (reverse l)))))
(def! last (fn* (l)
  (head (reverse l))))

(def! reduce
  ^{:d "f -> reduce func, b -> base, l -> list to reduce"}
  (fn* (f b l)
    (if (empty? l)
      b
      (reduce f (f b (head l)) (tail l)))))

(def! replicate
  ^{:d "returns a list filled with `n` of `replica`"}
  (fn* (replica n)
    (let* (
      repfn (fn* (l)
        (if (= (count l) n)
          l
          (repfn (cons replica l)))))
      (repfn []))))

(def! str-join
  ^{:d "takes a list of strings and a delimiter string and joins list items together"}
  (fn* (ls sep)
    (reduce str "")))

(def! file-folder
  (fn* (filename)
    (let* (
      splits (str-split filename *PATH_SEPARATOR*)
      appender (fn* (f s)
        (if (empty? s)
          f
          (if (eq (nth s (- (count s) 1) *PATH_SEPARATOR*))
            (appender (str f (head s)) (tail s))
            (appender (str f *PATH_SEPARATOR*) s)))))
      (appender "" (init splits)))))

(def! file-basename
  (fn* (filename)
    (last (str-split filename *PATH_SEPARATOR*))))

(def! file-ext
  (fn* (filename)
    (last (str-split filename *PATH_SEPARATOR*))))

(def! load-file
  (fn* (f)
    (do
      (let* (*FOLDER* (file-folder f))
        (println (str *FOLDER* *PATH_SEPARATOR* f))
        (eval (read-string
          (str "(do " (slurp (str *FOLDER* *PATH_SEPARATOR* f)) ")")))))))
