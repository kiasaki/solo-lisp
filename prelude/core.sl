(def! compose (fn* (f g)
  (fn* (& xs)
    (apply f (apply g xs)))))

(def! reduce
  (fn* (f in xs)
    (if (> (count xs) 0)
      (reduce f (f in (head xs)) (tail xs))
      in)))

(def! identity (fn* (x) x))

(def! reverse
  (fn* (xs)
    (reduce (fn* (ys x) (concat (list x) ys)) '() xs)))

(def! init (fn* (l)
  (reverse (tail (reverse l)))))
(def! last (fn* (l)
  (head (reverse l))))

(def! else true)

(defmacro! cond
  (fn* (& clauses)
    (if (> (count clauses) 0)
      (list 'if (head clauses)
        (if (> (count clauses) 1)
          (nth clauses 1)
          (throw "cond requires an even number of forms"))
        (cons 'cond (tail (tail clauses)))))))

(def! str-join (fn* (xs sep)
  (let* (
    appender (fn* (s ys flipflop)
      (cond
        (empty? ys) s
        flipflop (appender (str s sep) ys false)
        else (appender (str s (head ys)) (tail ys) true)))
    )
  (appender "" xs false))))

(def! file-folder (fn* (filename)
  (str-join (init (str-split filename *PATH_SEPARATOR*)) *PATH_SEPARATOR*)))

(def! load-file (fn* (f)
  (let* (
    filename (str *FOLDER* *PATH_SEPARATOR* f)
    *FOLDER* (file-folder filename))
    (println filename)
    (eval (read-string
      (str "(do " (slurp filename) ")")))
    nil)))
