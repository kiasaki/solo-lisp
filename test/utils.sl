; Constants
(def CURRENT_SUITE null)
(def CURRENT_TEST null)
(def TESTS_PASSED 0)
(def TESTS_FAILED 0)

; Helpers
(function logRed (text)
  (console.log (+ "\u001b[31m" text "\u001b[39m")))

(function logGrn (text)
  (console.log (+ "\u001b[32m" text "\u001b[39m")))

(function resetResults ()
  (set! TESTS_PASSED 0)
  (set! TESTS_FAILED 0))

(function printResults ()
  (def text (+ "PASSED: " TESTS_PASSED " FAILED: " TESTS_FAILED " TOTAL: "
    (+ TESTS_PASSED TESTS_FAILED)))

  (console.log)
  (console.log)

  (if (> TESTS_FAILED 0)
    (do
      (logRed text)
      (process.exit 1))
    (logGrn text)))

; Library public functions
(def suite (function (name block)
  (set! CURRENT_SUITE name)
  (block)
  (printResults)
  (resetResults)))

(def test (function (name block)
  (set! CURRENT_TEST name)
  (try
    (block)
    (set! TESTS_PASSED (+ 1 TESTS_PASSED))
    (process.stdout.write ".")
    (catch (e)
      (set! TESTS_FAILED (+ 1 TESTS_FAILED))
      (logRed (+ "FAIL [" CURRENT_SUITE "] " CURRENT_TEST))))))

(def assertEq (function (left right)
  (if (=== left right)
    null
    (throw (+ "Unexpected: " left " !== " right)))))

(def assertNotEq (function (left right)
  (if (!== left right)
    null
    (throw (+ "Unexpected: " left " === " right)))))

(set! exports.suite suite)
(set! exports.test test)
(set! exports.assertEq assertEq)
(set! exports.assertNotEq assertNotEq)
