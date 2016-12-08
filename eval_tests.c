#include <stdarg.h>
#include "builtins.h"
#include "eval.h"
#include "tests.h"

void evaltest_arith();
void evaltest_val_def();
void evaltest_fun_def();
void evaltest_rec_fun();
void evaltest_lambda();

void eval_tests() {
    evaltest_arith();
    evaltest_val_def();
    evaltest_fun_def();
    evaltest_rec_fun();
    evaltest_lambda();
}

// Forward declarations of testing utilities
void evaltest(char*, scamenv*, scamval* what_we_expect);
void evaltest_list(char*, scamenv*, int n, ...);
void evaldef(char*, scamenv*);

void evaltest_arith() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    evaltest("(+ 1 1)", env, scamint(2));
    evaltest("(- 10)", env, scamint(-10));
    evaltest("(- 10 3)", env, scamint(7));
    evaltest("(- 10 3.0)", env, scamdec(7.0));
    scamenv_free(env);
}

void evaltest_val_def() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    scamval_free(eval_line("(define x 10)", env));
    evaltest("x", env, scamint(10));
    evaltest("(* x 2)", env, scamint(20));
    evaltest("x", env, scamint(10));
    scamenv_free(env);
}

void evaltest_fun_def() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    evaldef("(define (square x) (* x x))", env);
    evaltest("(square 9)", env, scamint(81));
    evaltest("(square (square 3))", env, scamint(81));
    scamenv_free(env);
}

void evaltest_rec_fun() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    evaldef("(define (range i) (if (= i 0) [] (append (range (- i 1)) i)))", 
            env);
    evaltest_list("(range 5)", env, 5, scamint(1), scamint(2), scamint(3), 
                                       scamint(4), scamint(5));
    scamenv_free(env);
}

void evaltest_lambda() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    evaltest("((lambda (x y) (+ x y)) 20 22)", env, scamint(42));
    scamenv_free(env);
}

void evaltest(char* line, scamenv* env, scamval* what_we_expect) {
    scamval* what_we_got = eval_line(line, env);
    if (scamval_eq(what_we_got, what_we_expect)) {
        printf("Passed eval test \"%s\"\n", line);
    } else {
        printf("Failed eval test \"%s\"; expected ", line);
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
        scamval_append(items, v);
    }
    va_end(vlist);
    evaltest(line, env, items);
}

void evaldef(char* line, scamenv* env) {
    scamval* v = eval_line(line, env);
    if (v->type != SCAM_ERR) {
        printf("Passed eval test \"%s\"\n", line);
    } else {
        printf("Failed eval test \"%s\"\n", line);
        scamval_println(v);
    }
    scamval_free(v);
}
