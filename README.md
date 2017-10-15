# Scam

A toy functional programming language. Inspired by [SICP](https://mitpress.mit.edu/sicp/full-text/book/book.html) and [Build Your Own Lisp](http://www.buildyourownlisp.com/).


## Language features

All the regular Scheme stuff:

```racket
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

```racket
>>> (map square (list 1 2 3))
[1 4 9]
>>> ((lambda (x y) (+ x y)) 47 53)
100
```

Lists (actually, dynamic arrays) and dictionaries, with a convenient syntax inspired by Python:

```racket
>>> (concat ["Scheme" "Racket" "Common Lisp" "Clojure"] ["Scam"])
["Scheme" "Racket" "Common Lisp" "Clojure" "Scam"]
>>> (define dct {1:"one"  2:"two"})
>>> (get dct 1)
"one"
```

Lexical closures:

```racket
>>> (define (make-fun x) (lambda (y) (+ x y)))
>>> (define inc-by-10 (make-fun 10))
>>> (inc-by-10 30)
40
```

## How to compile

At present this project is highly non-portable. It will most likely only work with gcc, possibly only on Ubuntu. At a minimum, you will need to install readline, Flex and Bison. On Ubuntu this can be done with

```
$ sudo apt-get install libreadline6 libreadline6-dev flex bison
```

It is recommended you install [valgrind](http://valgrind.org/) (to run the test suite) and a LaTeX compiler (to read the docs), but these are not strictly necessary to compile the project.

Once these dependencies have been satisfied, simply run `make` in the root directory. An executable file `scam` will be created, which starts a REPL if run with no arguments. If you have valgrind installed, you can use the `test_all.sh` script to run the test suite.
