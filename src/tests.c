#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"
#include "scamval.h"

void parsetest(char* line, const scamval* answer, int line_no);

void evaltest(char* line, const scamval* answer, scamval* env, int line_no);
void evaltest_err(char* line, scamval* env, int line_no);

int main(int argc, char* argv[]) {
    #define PARSETEST(line, answer) parsetest(line, answer, __LINE__);
    #define EVALTEST(line, answer) evaltest(line, answer, env, __LINE__);
    #define EVALTEST_ERR(line) evaltest_err(line, env, __LINE__);
    #define EVALDEF(line) EVALTEST(line, scamnull());
    #define S scamsexpr_from
    #define L scamlist_from
    #define D scamdict_from
    puts("\n=== PARSER TESTS ===");
    puts("(you should see no failures)\n");

    /*** ATOMS ***/
    PARSETEST("174", scamint(174));
    PARSETEST("-78.3", scamdec(-78.3));
    PARSETEST("matador", scamsym("matador"));
    PARSETEST("\"matador\"", scamstr("matador"));

    puts("\n=== EVALUATOR TESTS ===");
    puts("(you should see two failed (+ 1 1) == 3 tests)\n");
    scamval* env = scamdict_builtins();

    /*** DEFINE ***/
    EVALTEST("(define x 42) x", scamint(42));
    EVALTEST("(define x 666) x", scamint(666));
    // test shadowing a global variable in a local scope
    EVALTEST("(define (redefine-x) (define x 13) x)  (redefine-x)", scamint(13));
    // test shadowing a global variable with a function parameter
    EVALTEST("(define (x-as-parameter x) x)  (x-as-parameter 17)", scamint(17));
    // test that values are immutable
    EVALTEST("(define items [1 2 3]) (tail items)", L(2, scamint(2), scamint(3)));
    EVALTEST("items", L(3, scamint(1), scamint(2), scamint(3)));
    // test bad defines
    EVALTEST_ERR("(define 1 1)");
    EVALTEST_ERR("(define (1) 1)");
    EVALTEST_ERR("(define (foo 10) 10)");
    EVALTEST_ERR("(define x)");
    EVALTEST_ERR("(define (foo x))");

    /*** RECURSION ***/
    EVALTEST("(define (countdown x) (if (= x 0) x (countdown (- x 1))))", scamnull());
    EVALTEST("(countdown 10)", scamint(0));
    // somewhat involved power function
    EVALDEF("(define (square x) (* x x))");
    EVALDEF("(define (power b n) (define (even? x) (= (% x 2) 0)) (if (= n 0) 1 (if (even? n) (square (power b (// n 2))) (* b (power b (- n 1))))))");
    EVALTEST("(power 287 0)", scamint(1));
    EVALTEST("(power 2 8)", scamint(256));
    EVALTEST("(power 17 8)", scamint(6975757441));

    /*** CLOSURES ***/
    EVALDEF("(define (make-fun x) (lambda (y) (+ x y)))");
    EVALTEST("((make-fun 10) 32)", scamint(42));
    // doubly nested closure
    EVALDEF("(define (make-fun1 x) (lambda (y) (lambda (z) (+ x y z))))");
    EVALTEST("(((make-fun1 1) 2) 3)", scamint(6));
    // another type of closure
    EVALDEF("(define (make-fun2) (define (double x) (* x 2)) double)");
    EVALDEF("(define foo (make-fun2))");
    EVALTEST("(foo 9)", scamint(18));
    // variables from the closure don't bleed into the global scope
    EVALTEST_ERR("double");

    /*** LAMBDA ***/
    EVALTEST("((lambda (x y) (+ x y)) 20 22)", scamint(42));
    EVALTEST("((lambda () (* 21 2)))", scamint(42));
    EVALTEST_ERR("((lambda (x y) (+ x y)) 20)");
    EVALTEST_ERR("((lambda (x y) (+ x y)) 20 21 22)");
    // parameters must be valid symbols
    EVALTEST_ERR("(lambda (10) (* 10 2))");
    EVALTEST_ERR("(lambda (x 10 y) (* x y))");
    // lambda expressions must have a body
    EVALTEST_ERR("(lambda (x))");

    /*** LIST and DICTIONARY LITERALS ***/
    EVALTEST("[(* 2 2) (* 3 3) (* 4 4)]", L(3, scamint(4), scamint(9), scamint(16)));
    EVALTEST("{1:\"one\"}", D(1, L(2, scamint(1), scamstr("one"))));

    /*** ARITHMETIC FUNCTIONS ***/
    // addition
    EVALTEST("(+ 7 -10 936 -14)", scamint(7 - 10 +936 - 14));
    EVALTEST("(+ 7.0 -10 936 -14)", scamdec(7 - 10 + 936 - 14));
    // negation and subtraction
    EVALTEST("(- 34.6)", scamdec(-34.6));
    EVALTEST("(- 347 80 -2 17)", scamint(347 - 80 + 2 - 17));
    EVALTEST("(- 347.0 80 -2 17)", scamdec(347 - 80 + 2 - 17));
    // multiplication
    EVALTEST("(* 9 9 -437)", scamint(9 * 9 * -437));
    EVALTEST("(* 3.5 6.79 2.3)", scamdec(3.5 * 6.79 * 2.3));
    // floating-point division
    EVALTEST("(/ 10 3)", scamdec(10 / 3.0));
    EVALTEST("(/ 3.7 8.91 2.3)", scamdec((3.7 / 8.91) / 2.3));
    EVALTEST_ERR("(/ 1 0)");
    EVALTEST_ERR("(/ 10 7 0 4)");
    // floor division
    EVALTEST("(// 10 3)", scamint(3));
    EVALTEST("(// -81 3 9)", scamint((-81 / 3) / 9));
    EVALTEST_ERR("(// 9 3.0)");
    EVALTEST_ERR("(// 1 0)");
    EVALTEST_ERR("(// 10 7 4 0)");
    // remainder
    EVALTEST("(% 10 3)", scamint(1));
    EVALTEST("(% -76 4 18)", scamint((-76 % 4) % 18));
    EVALTEST_ERR("(% 9.0 3)");
    EVALTEST_ERR("(% 1 0)");
    EVALTEST_ERR("(% 10 0 7 4)");

    /*** BOOLEAN OPERATORS ***/

    /*** COMPARISON AND EQUALITY ***/
    // numeric equality
    EVALTEST("(= 1 1)", scambool(1));
    EVALTEST("(= -81 -81.0)", scambool(1));
    EVALTEST("(= 1 0.99999)", scambool(0));
    EVALTEST("(= 1 \"1\")", scambool(0));
    // string equality
    EVALTEST("(=  \"money\"  \"money\")", scambool(1));
    EVALTEST("(=  \"lucre\"  \" lucre \")", scambool(0));
    EVALTEST("(=  \"\"  \"\")", scambool(1));
    // boolean equality
    EVALTEST("(= true true)", scambool(1));
    EVALTEST("(= false false)", scambool(1));
    EVALTEST("(= true false)", scambool(0));
    EVALTEST("(= false true)", scambool(0));
    // list equality
    EVALTEST("(= [] [])", scambool(1));
    EVALTEST("(= [\"S\" [\"NP\" \"VP\"]] [\"S\" [\"VP\" \"NP\"]])", scambool(0));
    EVALTEST("(= [1 2 3 4 5] [1 2 3 4 5])", scambool(1));

    /*** LIST FUNCTIONS ***/
    EVALTEST("[1 2 3 4 5]", L(5, scamint(1), scamint(2), scamint(3), scamint(4), scamint(5)));
    EVALTEST("[[\"inception\"]]", L(1, L(1, scamstr("inception"))));

    /*** IO FUNCTIONS ***/
    EVALDEF("(define fp (open \"resources/foo.txt\" \"r\"))");
    EVALTEST("(port-good? fp)", scambool(1));
    EVALTEST("(readline fp)", scamstr("Lorem ipsum\n"));
    EVALTEST("(port-good? fp)", scambool(1));
    EVALTEST_ERR("(readline fp)");
    EVALTEST("(port-good? fp)", scambool(0));
    EVALDEF("(close fp)");

    /*** INTENTIONAL FAIL ***/
    EVALTEST("(+ 1 1)", scamint(3));
    EVALTEST_ERR("(+ 1 1)");
    gc_close();
    return 0;
}

void parsetest(char* line, const scamval* answer, int line_no) {
    scamval* v = parse_str(line);
    scamval* modified_answer = scamsexpr_from(2, scamsym("begin"), answer);
    if (!scamval_eq(v, modified_answer)) {
        printf("Failed parse example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ");
        scamval_println(modified_answer);
        printf("Got:\n  ");
        scamval_println(v);
        printf("\n");
    }
}

void evaltest(char* line, const scamval* answer, scamval* env, int line_no) {
    scamval* v = eval_str(line, env);
    if (!scamval_eq(v, answer)) {
        printf("Failed example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ");
        scamval_println(answer);
        printf("Got:\n  ");
        scamval_println(v);
        printf("\n");
    } else if (v->type != answer->type) {
        printf("Failed example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ");
        printf("  %s\n", scamtype_debug_name(answer->type));
        printf("Got:\n  ");
        printf("  %s\n\n", scamtype_debug_name(v->type));
    }
    gc_unset_root(v);
}

void evaltest_err(char* line, scamval* env, int line_no) {
    scamval* v = eval_str(line, env);
    if (v->type != SCAM_ERR) {
        printf("Failed example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ERROR\n");
        printf("Got:\n  ");
        scamval_println(v);
        printf("\n");
    }
    gc_unset_root(v);
}
