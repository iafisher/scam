#include <stdarg.h>
#include "builtins.h"
#include "eval.h"
#include "tests.h"

// Forward declarations of testing utilities
void evaltest(char*, scamenv*, scamval* what_we_expect);
void evaltest_list(char*, scamenv*, int n, ...);
void evaldef(char*, scamenv*);

void eval_tests() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    // test basic arithmetic
    evaltest("(+ 1 1)", env, scamval_int(2));
    // test variable definition
    scamval_free(eval_line("(define x 10)", env));
    evaltest("x", env, scamval_int(10));
    evaltest("(* x 2)", env, scamval_int(20));
    evaltest("x", env, scamval_int(10));
    // test function definition
    evaldef("(define square (lambda (x) (* x x)))", env);
    evaltest("(square 9)", env, scamval_int(81));
    evaltest("(square (square 3))", env, scamval_int(81));
    // test recursive function definition
    evaldef("(define range (lambda (i) (if (= i 0) [] (append (range (- i 1)) i))))", env);
    evaltest_list("(range 5)", env, 5, scamval_int(1), scamval_int(2), 
                  scamval_int(3), scamval_int(4), scamval_int(5));
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
    scamval* items = scamval_list();
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
