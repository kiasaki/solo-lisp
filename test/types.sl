(import
  ("./utils" refer (suite test assertEq)))

(suite "types" (function ()

  (test "strings" (function ()
    (assertEq "a" "a")))

  (test "numbers" (function ()
    (assertEq 0 0)
    (assertEq 5 5)))))
