;;;
;;;   Solo Standard Prelude
;;;

;;; Atoms
(def 'nil '())
(def 'true 1)
(def 'false 0)

;;; Functional Functions

; Function Definitions
(def 'defn (fn '(f b) '(
  def (head f) (fn (tail f) b)
)))
