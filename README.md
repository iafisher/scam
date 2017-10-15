# Scam

A toy functional programming language: Scheme, but on a budget.



## Language features

All the regular Scheme stuff:

```scheme
>>> (+ 1 1)
2
>>> (define x "Hello, world")
>>> x
"Hello, world"
>>> (define (square x) (* x x))
>>> (square 9)
81
```

First-class functions:

```scheme
>>> (map square (list 1 2 3))
[1 4 9]
>>> ((lambda (x y) (+ x y)) 47 53)
100
```

Lists (actually, dynamic arrays) and dictionaries, with a convenient syntax inspired by Python:

```scheme
>>> (concat ["Scheme" "Racket" "Common Lisp" "Clojure"] ["Scam"])
["Scheme" "Racket" "Common Lisp" "Clojure" "Scam"]
>>> (define dct {1:"one"  2:"two"})
>>> (get dct 1)
"one"
```

Lexical closures:

```scheme
>>> (define (make-fun x) (lambda (y) (+ x y)))
>>> (define inc-by-10 (make-fun 10))
>>> (inc-by-10 30)
40
```

