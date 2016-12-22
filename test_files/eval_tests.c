#include <stdarg.h>
#include "../collector.h"
#include "../eval.h"
#include "tests.h"

void evaltest_val_def(scamval*);
void evaltest_fun_def(scamval*);
void evaltest_closure(scamval*);
void evaltest_rec_fun(scamval*);
void evaltest_lambda(scamval*);

void eval_tests() {
    scamval* env = scamdict_builtins();
    evaltest_val_def(env);
    evaltest_fun_def(env);
    evaltest_rec_fun(env);
    evaltest_lambda(env);
    evaltest_closure(env);
    gc_close();
}

// Forward declarations of testing utilities
void evaltest(char*, scamval*, scamval* what_we_expect);
void evaltest_list(char*, scamval*, int n, ...);
void evaltest_err(char*, scamval*);
void evaldef(char*, scamval*);
void evaltrue(char*, scamval*);
void evalfalse(char*, scamval*);

void evaltest_val_def(scamval* env) {
    evaldef("(define x 10)", env);
    evaltest("x", env, scamint(10));
    evaltest("(* x 2)", env, scamint(20));
    evaltest("x", env, scamint(10));
    // test redefinition
    evaldef("(define x 8)", env);
    evaltest("x", env, scamint(8));
    evaltest_err("(define 1 23)", env);
}

void evaltest_fun_def(scamval* env) {
    evaldef("(define (countdown x) (if (= x 0) x (countdown (- x 1))))", env);
    evaltest("(countdown 10)", env, scamint(0));
    // basic square function
    evaldef("(define (square x) (* x x))", env);
    evaltest("(square 9)", env, scamint(81));
    evaltest("(square (square 3))", env, scamint(81));
    evaltest_err("(square)", env);
    evaltest_err("(square 2 3)", env);
    evaltest_err("(square [])", env);
    // more complicated power function
    evaldef("(define (power b n) (define (even? x) (= (% x 2) 0)) (if (= n 0) 1 (if (even? n) (square (power b (// n 2))) (* b (power b (- n 1))))))", env);
    evaltest("(power 287 0)", env, scamint(1));
    evaltest("(power 2 2)", env, scamint(4));
    evaltest("(power 2 8)", env, scamint(256));
    evaltest("(power 17 8)", env, scamint(6975757441));
    evaltest_err("even?", env);
}

void evaltest_closure(scamval* env) {
    // single nested closure
    evaldef("(define (make-fun x) (lambda (y) (+ x y)))", env);
    evaldef("(define foo (make-fun 5))", env);
    evaltest("(foo 37)", env, scamint(42));
    // doubly nested closure
    evaldef("(define (make-fun1 x) (lambda (y) (lambda (z) (+ x y z))))", env);
    evaldef("(define foo (make-fun1 5))", env);
    evaldef("(define bar (foo 32))", env);
    evaltest("(bar 5)", env, scamint(42));
    // a different kind of closure
    evaldef("(define (make-fun2) (define (double x) (* x 2)) double)", env);
    evaldef("(define foo (make-fun2))", env);
    evaltest("(foo 9)", env, scamint(18));
    evaltest_err("double", env);
}

void evaltest_rec_fun(scamval* env) {
    // recursive range function (range1 because range is a builtin name)
    evaldef("(define (range1 i) (if (= i 0) [] (append (range1 (- i 1)) i)))", 
            env);
    evaltest_list("(range1 5)", env, 5, scamint(1), scamint(2), scamint(3), 
                                        scamint(4), scamint(5));
    // naive recursive Fibonacci function
    evaldef("(define (fib i) (if (< i 3) 1 (+ (fib (- i 1)) (fib (- i 2)))))",
            env);
    evaltest("(fib 1)", env, scamint(1));
    evaltest("(fib 2)", env, scamint(1));
    evaltest("(fib 3)", env, scamint(2));
    evaltest("(fib 12)", env, scamint(144));
}

void evaltest_lambda(scamval* env) {
    evaltest("((lambda (x y) (+ x y)) 20 22)", env, scamint(42));
    evaltest_err("((lambda (x y) (+ x y)) 20)", env);
    evaltest_err("((lambda (x y) (+ x y)) 20 21 22)", env);
    // make sure parameters must be valid symbols
    evaltest_err("(lambda (10) (* 10 2))", env);
    evaltest_err("(lambda (x 10 y) (* x y))", env);
}

void evaltest(char* line, scamval* env, scamval* what_we_expect) {
    scamval* what_we_got = eval_str(line, env);
    if (!scamval_eq(what_we_got, what_we_expect)) {
        printf("Failure at %s:%d ", __FILE__, __LINE__);
        printf("(test \"%s\"); expected ", line);
        scamval_print_debug(what_we_expect);
        printf(" got ");
        scamval_print_debug(what_we_got);
        printf("\n");
    }
    gc_unset_root(what_we_got);
    gc_unset_root(what_we_expect);
}

void evaltest_list(char* line, scamval* env, int n, ...) {
    va_list vlist;
    va_start(vlist, n);
    scamval* items = scamlist();
    for (int i = 0; i < n; i++) {
        scamval* v = va_arg(vlist, scamval*);
        scamseq_append(items, v);
    }
    va_end(vlist);
    evaltest(line, env, items);
}

void evaltest_err(char* line, scamval* env) {
    scamval* v = eval_str(line, env);
    if (v->type != SCAM_ERR) {
        printf("Failure at %s:%d ", __FILE__, __LINE__);
        printf("(test \"%s\"), expected error\n", line);
    }
    gc_unset_root(v);
}

void evaldef(char* line, scamval* env) {
    scamval* v = eval_str(line, env);
    if (v->type == SCAM_ERR) {
        printf("Failure at %s:%d ", __FILE__, __LINE__);
        printf("(test \"%s\")\n", line);
        scamval_println(v);
    }
    gc_unset_root(v);
}

void evaltrue(char* line, scamval* env) {
    evaltest(line, env, scambool(1));
}

void evalfalse(char* line, scamval* env) {
    evaltest(line, env, scambool(0));
}
