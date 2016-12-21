#include <stdarg.h>
#include "../builtins.h"
#include "../eval.h"
#include "tests.h"

void evaltest_val_def(scamenv*);
void evaltest_fun_def(scamenv*);
void evaltest_closure(scamenv*);
void evaltest_rec_fun(scamenv*);
void evaltest_lambda(scamenv*);
void evaltest_zero_div(scamenv*);
void evaltest_known_fails(scamenv*);

void eval_tests() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    evaltest_val_def(env);
    evaltest_fun_def(env);
    evaltest_rec_fun(env);
    evaltest_lambda(env);
    evaltest_zero_div(env);
    evaltest_closure(env);
    //evaltest_known_fails(env);
    scamenv_free(env);
}

// Forward declarations of testing utilities
void evaltest(char*, scamenv*, scamval* what_we_expect);
void evaltest_list(char*, scamenv*, int n, ...);
void evaltest_err(char*, scamenv*);
void evaldef(char*, scamenv*);
void evaltrue(char*, scamenv*);
void evalfalse(char*, scamenv*);

void evaltest_val_def(scamenv* env) {
    evaldef("(define x 10)", env);
    evaltest("x", env, scamint(10));
    evaltest("(* x 2)", env, scamint(20));
    evaltest("x", env, scamint(10));
    // test redefinition
    evaldef("(define x 8)", env);
    evaltest("x", env, scamint(8));
    evaltest_err("(define 1 23)", env);
}

void evaltest_fun_def(scamenv* env) {
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

void evaltest_closure(scamenv* env) {
    // single nested closure
    evaldef("(define (make-fun x) (lambda (y) (+ x y)))", env);
    evaldef("(define foo (make-fun 5))", env);
    evaltest("(foo 37)", env, scamint(42));
    // doubly nested closure
    evaldef("(define (make-fun1 x) (lambda (y) (lambda (z) (+ x y z))))", env);
    evaldef("(define foo (make-fun1 5))", env);
    evaldef("(define bar (foo 32))", env);
    evaltest("(bar 5)", env, scamint(42));
}

void evaltest_known_fails(scamenv* env) {
    evaldef("(define (make-fun2) (define (double x) (* x 2)) double)", env);
    evaldef("(define foo (make-fun2))", env);
    evaltest("(foo 9)", env, scamint(18));
}

void evaltest_rec_fun(scamenv* env) {
    evaldef("(define (range1 i) (if (= i 0) [] (append (range1 (- i 1)) i)))", 
            env);
    evaltest_list("(range1 5)", env, 5, scamint(1), scamint(2), scamint(3), 
                                       scamint(4), scamint(5));
}

void evaltest_lambda(scamenv* env) {
    evaltest("((lambda (x y) (+ x y)) 20 22)", env, scamint(42));
    evaltest_err("((lambda (x y) (+ x y)) 20)", env);
    evaltest_err("((lambda (x y) (+ x y)) 20 21 22)", env);
    // make sure parameters must be valid symbols
    evaltest_err("(lambda (10) (* 10 2))", env);
    evaltest_err("(lambda (x 10 y) (* x y))", env);
}

void evaltest_zero_div(scamenv* env) {
    evaltest_err("(/ 10 0)", env);
    evaltest_err("(/ 10 23 0)", env);
    evaltest_err("(+ 10 (/ 10 0))", env);
    evaltest_err("(// 10 0)", env);
    evaltest_err("(% 10 0)", env);
}

void evaltest(char* line, scamenv* env, scamval* what_we_expect) {
    scamval* what_we_got = eval_str(line, env);
    if (!scamval_eq(what_we_got, what_we_expect)) {
        printf("Failure at %s:%d ", __FILE__, __LINE__);
        printf("(test \"%s\"); expected ", line);
        scamval_print_debug(what_we_expect);
        printf(" got ");
        scamval_print_debug(what_we_got);
        printf("\n");
    }
    scamval_free(what_we_got);
    scamval_free(what_we_expect);
}

void evaltest_list(char* line, scamenv* env, int n, ...) {
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

void evaltest_err(char* line, scamenv* env) {
    scamval* v = eval_str(line, env);
    if (v->type != SCAM_ERR) {
        printf("Failure at %s:%d ", __FILE__, __LINE__);
        printf("(test \"%s\"), expected error\n", line);
    }
    scamval_free(v);
}

void evaldef(char* line, scamenv* env) {
    scamval* v = eval_str(line, env);
    if (v->type == SCAM_ERR) {
        printf("Failure at %s:%d ", __FILE__, __LINE__);
        printf("(test \"%s\")\n", line);
        scamval_println(v);
    }
    scamval_free(v);
}

void evaltrue(char* line, scamenv* env) {
    evaltest(line, env, scambool(1));
}

void evalfalse(char* line, scamenv* env) {
    evaltest(line, env, scambool(0));
}
