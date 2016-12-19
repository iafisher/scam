#include <stdarg.h>
#include "builtins.h"
#include "eval.h"
#include "tests.h"

void evaltest_arith(scamenv*);
void evaltest_cmp(scamenv*);
void evaltest_lists(scamenv*);
void evaltest_val_def(scamenv*);
void evaltest_fun_def(scamenv*);
void evaltest_closure(scamenv*);
void evaltest_rec_fun(scamenv*);
void evaltest_lambda(scamenv*);
void evaltest_zero_div(scamenv*);

void eval_tests() {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    evaltest_arith(env);
    evaltest_cmp(env);
    evaltest_lists(env);
    evaltest_val_def(env);
    evaltest_fun_def(env);
    evaltest_rec_fun(env);
    evaltest_lambda(env);
    evaltest_zero_div(env);
    //evaltest_closure(env);
    scamenv_free(env);
}

// Forward declarations of testing utilities
void evaltest(char*, scamenv*, scamval* what_we_expect);
void evaltest_list(char*, scamenv*, int n, ...);
void evaltest_err(char*, scamenv*);
void evaldef(char*, scamenv*);
void evaltrue(char*, scamenv*);
void evalfalse(char*, scamenv*);

void evaltest_arith(scamenv* env) {
    // test addition
    evaltest("(+ 1 1)", env, scamint(2));
    evaltest("(+ 1 2 3 4 5)", env, scamint(15));
    evaltest("(+ 1 2 3.0 4 5)", env, scamint(15.0));
    evaltest_err("(+)", env);
    evaltest_err("(+ 1)", env);
    evaltest_err("(+ 1 [])", env);
    // test negation and subtraction
    evaltest("(- 10)", env, scamint(-10));
    evaltest("(- 10 3)", env, scamint(7));
    evaltest("(- 10 3.0)", env, scamdec(7.0));
    evaltest("(- 10 8 2 3)", env, scamint(-3));
    evaltest_err("(-)", env);
    evaltest_err("(- [])", env);
    evaltest_err("(- 1 [])", env);
    // test multiplication
    evaltest("(* 21 2)", env, scamint(42));
    evaltest("(* 3.2 7.4)", env, scamdec(3.2 * 7.4));
    evaltest("(* 1 2 3 4 5 6)", env, scamint(720));
    evaltest_err("(*)", env);
    evaltest_err("(* 1)", env);
    evaltest_err("(* 1 [])", env);
    // test real division
    evaltest("(/ 10 2)", env, scamdec(5.0));
    evaltest("(/ -72 2.2)", env, scamdec(-72 / 2.2));
    evaltest("(/ 0 42)", env, scamdec(0.0));
    evaltest_err("(/ 10 0)", env);
    evaltest_err("(/ 10 23 0)", env);
    evaltest_err("(+ 10 (/ 10 0))", env);
    evaltest_err("(/)", env);
    evaltest_err("(/ 10)", env);
    evaltest_err("(/ 10 \"abc\")", env);
    // test floor division
    evaltest("(// 10 3)", env, scamint(3));
    evaltest("(// 50 11 2)", env, scamint(2));
    evaltest("(// 0 42)", env, scamint(0.0));
    evaltest_err("(// 10 3.0)", env);
    evaltest_err("(// 10 0)", env);
    evaltest_err("(//)", env);
    evaltest_err("(// 10)", env);
    evaltest_err("(// 10 \"abc\")", env);
    // test remainder
    evaltest("(% 73 2)", env, scamint(1));
    evaltest("(% 67 7)", env, scamint(67 % 7));
    evaltest("(% 0 10)", env, scamint(0));
    evaltest_err("(% 42 4.7)", env);
    evaltest_err("(% 10 0)", env);
    evaltest_err("(%)", env);
    evaltest_err("(% 10)", env);
    evaltest_err("(% 10 \"abc\")", env);
}

void evaltest_cmp(scamenv* env) {
    // test numeric =
    evaltrue("(= 1 1)", env);
    evaltrue("(= 1 1.0)", env);
    evaltrue("(= 1.0 1)", env);
    evalfalse("(= 1  0.999)", env);
    evalfalse("(= 10 \"10\")", env);
    // test string =
    evaltrue("(= \"abc\" \"abc\")", env);
    evaltrue("(= \"\" \"\")", env);
    evalfalse("(= \"hello\" \"hallo\")", env);
    evalfalse("(= \"short\" \"a longer string\")", env);
    // test list =
    evaltrue("(= [1 2 3] [1 2 3])", env);
    evaltrue("(= [1 2 3] [1.0 2.0 3.0])", env);
    evaltrue("(= [] [])", env);
    evaltrue("(= [\"a string\" 10 true] [\"a string\" 10 true])", env);
    evalfalse("(= [1 2 3] [1 2 3 4])", env);
}

void evaltest_lists(scamenv* env) {
    evaltest("(empty? [])", env, scambool(1));
    evaltest_err("(empty?)", env);
    evaltest_err("(empty? 10)", env);
    // extended example
    evaldef("(define items [1 2 3 4 5 6])", env);
    evaltest("(len items)", env, scamint(6));
    evaltest("(empty? items)", env, scambool(0));
    evaltest("(head items)", env, scamint(1));
    evaltest_list("(tail items)", env, 5, scamint(2), scamint(3), scamint(4),
                                          scamint(5), scamint(6));
    evaltest("(last items)", env, scamint(6));
    evaltest_list("(init items)", env, 5, scamint(1), scamint(2), scamint(3), 
                                          scamint(4), scamint(5));
    evaltest_list("(prepend 0 items)", env, 7, scamint(0), scamint(1), 
                  scamint(2), scamint(3), scamint(4), scamint(5), scamint(6));
    evaltest_list("(append items 7)", env, 7, scamint(1), scamint(2), 
                  scamint(3), scamint(4), scamint(5), scamint(6), scamint(7));
}

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
    evaldef("(define (square x) (* x x))", env);
    evaltest("(square 9)", env, scamint(81));
    evaltest("(square (square 3))", env, scamint(81));
    evaltest_err("(square)", env);
    evaltest_err("(square 2 3)", env);
    evaltest_err("(square [])", env);
}

void evaltest_closure(scamenv* env) {
    // single nested closure
    evaldef("(define (make-fun x) (lambda (y) (+ x y)))", env);
    evaldef("(define foo (make-fun 5))", env);
    evaltest("(foo 37)", env, scamint(42));
    // doubly nested closure
    evaldef("(define (make-fun x) (lambda (y) (lambda (z) (+ x y z))))", env);
    evaldef("(define foo (make-fun 5))", env);
    evaldef("(define bar (foo 32))", env);
    evaltest("(bar 5)", env, scamint(42));
}

void evaltest_rec_fun(scamenv* env) {
    evaldef("(define (range i) (if (= i 0) [] (append (range (- i 1)) i)))", 
            env);
    evaltest_list("(range 5)", env, 5, scamint(1), scamint(2), scamint(3), 
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
}

void evaltest(char* line, scamenv* env, scamval* what_we_expect) {
    scamval* what_we_got = eval_str(line, env);
    if (!scamval_eq(what_we_got, what_we_expect)) {
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
        scamseq_append(items, v);
    }
    va_end(vlist);
    evaltest(line, env, items);
}

void evaltest_err(char* line, scamenv* env) {
    scamval* v = eval_str(line, env);
    if (v->type != SCAM_ERR) {
        printf("Failed eval test \"%s\" (expected error)\n", line);
    }
    scamval_free(v);
}

void evaldef(char* line, scamenv* env) {
    scamval* v = eval_str(line, env);
    if (v->type == SCAM_ERR) {
        printf("Failed eval test \"%s\"\n", line);
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
