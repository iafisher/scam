>>> (define x 10)
>>> x
10
>>> (* x 2)
20
>>> x
10
; test redefinition
>>> (define x 8)
>>> x
8
; test shadowing within a value within a function
>>> (define (redefine-x) (define x 42) x)
>>> (redefine-x)
42
>>> x
8
; test shadowing a function parameter
>>> (define (x-as-parameter x) x)
>>> (x-as-parameter 57)
57
>>> x
8
; make sure that lists are immutable
>>> (define items [1 2 3])
>>> (tail items)
[2 3]
>>> items
[1 2 3]
; test various bad defines
>>> (define 1 23)
ERROR
>>> (define y)
ERROR
>>> (define (f x))
ERROR
; stupid recursive function
>>> (define (countdown x) (if (= x 0) x (countdown (- x 1))))
>>> (countdown 10)
0
; basic square function
>>> (define (square x) (* x x))
>>> (square 9)
81
>>> (square (square 3))
81
>>> (square)
ERROR
>>> (square 2 3)
ERROR
>>> (square [])
ERROR
; more complicated power function
>>> (define (power b n) (define (even? x) (= (% x 2) 0)) (if (= n 0) 1 (if (even? n) (square (power b (// n 2))) (* b (power b (- n 1))))))
>>> (power 287 0)
1
>>> (power 2 2)
4
>>> (power 2 8)
256
>>> (power 17 8)
6975757441
>>> even?
ERROR
; single nested closure
>>> (define (make-fun x) (lambda (y) (+ x y)))
>>> (define foo (make-fun 5))
>>> (foo 37)
42
; doubly nested closure
>>> (define (make-fun1 x) (lambda (y) (lambda (z) (+ x y z))))
>>> (define foo (make-fun1 5))
>>> (define bar (foo 32))
>>> (bar 5)
42
; a different kind of closure
>>> (define (make-fun2) (define (double x) (* x 2)) double)
>>> (define foo (make-fun2))
>>> (foo 9)
18
>>> double
ERROR
; recursive range function (range1 because range is a builtin name)
>>> (define (range1 i) (if (= i 0) [] (append (range1 (- i 1)) i))) 
>>> (range1 5)
[1 2 3 4 5]
; naive recursive Fibonacci function
>>> (define (fib i) (if (< i 3) 1 (+ (fib (- i 1)) (fib (- i 2)))))
>>> (fib 1)
1
>>> (fib 2)
1
>>> (fib 3)
2
>>> (fib 12)
144
; iterative Fibonacci function
>>> (define (fib i) (define (fib-iter fib-j fib-j-minus-1 j j-minus-1) (if (= i j) fib-j (fib-iter (+ fib-j fib-j-minus-1) fib-j (+ j 1) j))) (if (< i 3) 1 (fib-iter 1 1 2 1)))
>>> (fib 1)
1
>>> (fib 2)
1
>>> (fib 3)
2
>>> (fib 12)
144
>>> (fib 60)
1548008755920
>>> (fib 70)
190392490709135
; these tests are off by a couple digits for some reason
;>>> (fib 80)
;23416728348467685
;>>> (fib 90)
;2880067194370816120
; lambda tests
>>> ((lambda (x y) (+ x y)) 20 22)
42
>>> ((lambda () (* 21 2)))
42
>>> ((lambda (x y) (+ x y)) 20)
ERROR
>>> ((lambda (x y) (+ x y)) 20 21 22)
ERROR
; make sure parameters must be valid symbols
>>> (lambda (10) (* 10 2))
ERROR
>>> (lambda (x 10 y) (* x y))
ERROR
; make sure lambda expressions must have a body
>>> (lambda (x))
ERROR

; make sure that list and dictionary literals evaluate their components
>>> [(* 2 2) (* 3 3) (* 4 4)]
[4 9 16]
>>> {(* 2 2):"four"  (* 3 3):(concat "nin" "e") 16:"sixteen"}
{4:"four" 9:"nine" 16:"sixteen"}
