; solo lisp test
(require "asd\n\x41")

(+ 1 2)

(set asd {a 1 b 2})

(function ()
  (if true
    false
    9))

(console.log "asd")
