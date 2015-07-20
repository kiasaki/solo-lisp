(import
  ("./utils" refer (suite test assertEq)))

(suite "types" (function ()

  (test "strings" (function ()
    (assertEq "a" "a")))))
