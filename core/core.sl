; Simple definition of functions
; def {add} (\ {x y} {+ x y}) -> fun {add x y} {+ x y}
def {fun} (\ {args body} {def (head args) (\ (tail args) body)})

; Prepends an atom to a list
fun {cons a l} {join (list a) l}

; Calls a function with a list of params instead of params individually
; (func-to-call x y z) -> ((unpack func-to-call) (x y z))
fun {unpack f xs} {eval (join (list f) xs)}
def {curry} unpack

; Call a function expecting a list with individually passed in params
; (func-to-call (x y z)) -> ((pack func-to-call) x y z)
fun {pack f xs...} {f xs}
def {uncurry} pack
