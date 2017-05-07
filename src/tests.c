#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"
#include "scamval.h"

void parsetest(char* line, const scamval* answer, int line_no);
void parsetest_err(char* line, int line_no);

void evaltest(char* line, const scamval* answer, scamval* env, int line_no);
void evaltest_err(char* line, scamval* env, int line_no);

int main(int argc, char* argv[]) {
    #define PARSETEST(line, answer) parsetest(line, answer, __LINE__);
    #define PARSETEST_ERR(line) parsetest_err(line, __LINE__);
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
    PARSETEST("(+ 1 1)", S(3, scamsym("+"), scamint(1), scamint(1)));
    PARSETEST("[1 2 3]", S(4, scamsym("list"), scamint(1), scamint(2), scamint(3)));
    PARSETEST("{1:\"one\"}", S(2, scamsym("dict"), 
                                  S(3, scamsym("list"), scamint(1), scamstr("one"))));
    PARSETEST_ERR("(+ (define x 10) 3)");

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
    EVALTEST("(and true true)", scambool(1));
    EVALTEST("(and true false)", scambool(0));
    EVALTEST("(and false true)", scambool(0));
    EVALTEST("(and false false)", scambool(0));
    EVALTEST("(and true true true true false)", scambool(0));
    EVALTEST("(or true true)", scambool(1));
    EVALTEST("(or true false)", scambool(1));
    EVALTEST("(or false true)", scambool(1));
    EVALTEST("(or false false)", scambool(0));
    EVALTEST("(not true)", scambool(0));
    EVALTEST("(not false)", scambool(1));
    // test short-circuiting
    EVALTEST("(and false (begin (/ 10 0) true))", scambool(0));
    EVALTEST_ERR("(and (begin (/ 10 0) true) false)");
    EVALTEST("(or true (begin (/ 10 0) true))", scambool(1));
    EVALTEST_ERR("(or (begin (/ 10 0) false) true)");

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
    // empty?
    EVALTEST("(empty? [])", scambool(1));
    EVALTEST("(empty? [[]])", scambool(0));
    // len
    EVALTEST("(len [])", scamint(0));
    EVALTEST("(len [1 2 3])", scamint(3));
    EVALTEST("(len [1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24])", scamint(24));
    // head
    EVALTEST("(head [1 2 3])", scamint(1));
    EVALTEST_ERR("(head [])");
    // tail
    EVALTEST("(tail [1 2 3])", L(2, scamint(2), scamint(3)));
    EVALTEST("(tail [])", scamlist());
    // last
    EVALTEST("(last [1 2 3])", scamint(3));
    EVALTEST_ERR("(last [])");
    // prepend
    EVALTEST("(prepend 0 [1 2 3])", L(4, scamint(0), scamint(1), scamint(2), scamint(3)));
    EVALTEST("(prepend 0 [])", L(1, scamint(0)));
    // append
    EVALTEST("(append [1 2 3] 4)", L(4, scamint(1), scamint(2), scamint(3), scamint(4)));
    EVALTEST("(append [] 0)", L(1, scamint(0)));
    // concat
    EVALTEST("(concat [1 2] [3 4])", L(4, scamint(1), scamint(2), scamint(3), scamint(4)));
    EVALTEST("(concat [1 2 3] [])", L(3, scamint(1), scamint(2), scamint(3)));
    EVALTEST("(concat [] [1 2 3])", L(3, scamint(1), scamint(2), scamint(3)));
    EVALTEST("(concat [] [])", scamlist());
    EVALTEST_ERR("(concat [] \"\")");
    // get
    EVALTEST("(get [\"I\" \"met\" \"a\" \"traveller\" \"from\" \"an\" \"antique\" \"land\"] 2)",
             scamstr("a"));
    EVALTEST("(get [\"Fort Sumter\" \"Antietam\" \"Gettysburg\"] 2)", scamstr("Gettysburg"));
    EVALTEST_ERR("(get [1] 1)");
    EVALTEST_ERR("(get [1] -1)");
    EVALTEST_ERR("(get [] 0)");
    // slice
    EVALTEST("(slice [1 2 3 4 5 6 7 8 9] 3 6)", L(3, scamint(4), scamint(5), scamint(6)));
    EVALTEST("(slice [1 2 3] 0 3)", L(3, scamint(1), scamint(2), scamint(3)));
    EVALTEST("(slice [1 2 3] 0 0)", scamlist());
    EVALTEST_ERR("(slice [1 2 3] 0 4)");
    EVALTEST_ERR("(slice [1 2 3] -1 3)");
    EVALTEST_ERR("(slice [1 2 3] 2 1)");
    // take
    EVALTEST("(take [1 2 3 4 5 6 7] 3)", L(3, scamint(1), scamint(2), scamint(3)));
    EVALTEST("(take [] 0)", scamlist());
    EVALTEST("(take [1 2 3] 3)", L(3, scamint(1), scamint(2), scamint(3)));
    EVALTEST_ERR("(take [1 2 3] 4)");
    // drop
    EVALTEST("(drop [1 2 3 4 5 6 7] 3)", L(4, scamint(4), scamint(5), scamint(6), scamint(7)));
    EVALTEST("(drop [] 0)", scamlist());
    EVALTEST("(drop [1 2 3] 3)", scamlist());
    EVALTEST_ERR("(drop [1 2 3] 4)");
    // insert
    EVALTEST("(insert [\"one\" \"small\" \"step\" \"for\" \"man\"] 4 \"a\")",
             L(6, scamstr("one"), scamstr("small"), scamstr("step"), scamstr("for"), scamstr("a"),
                  scamstr("man")));
    EVALTEST("(insert [1 2 3] 0 0)", L(4, scamint(0), scamint(1), scamint(2), scamint(3)));
    EVALTEST("(insert [1 2 3] 3 4)", L(4, scamint(1), scamint(2), scamint(3), scamint(4)));
    EVALTEST("(insert [] 0 \"first\")", L(1, scamstr("first")));
    // find
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Germanic\")", scamint(0));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Slavic\")", scamint(1));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Romance\")", scamint(2));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Indo-Iranian\")", scambool(0));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"romance\")", scambool(0));
    EVALTEST("(find [\"duplicate\" \"different\" \"duplicate\"] \"duplicate\")", scamint(0));
    // rfind
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Germanic\")", scamint(0));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Slavic\")", scamint(1));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Romance\")", scamint(2));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Indo-Iranian\")", scambool(0));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"romance\")", scambool(0));
    EVALTEST("(rfind [\"duplicate\" \"different\" \"duplicate\"] \"duplicate\")", scamint(2));
    // sort
    EVALTEST("(sort [5 4 3 2 1])", 
             L(5, scamint(1), scamint(2), scamint(3), scamint(4), scamint(5)));
    // map
    EVALTEST("(map (lambda (x) (* x 2)) [1 2 3 4 5])",
             L(5, scamint(2), scamint(4), scamint(6), scamint(8), scamint(10)));
    EVALTEST_ERR("(map (lambda (x y) (+ x y)) [1 2 3 4 5 ])");
    // filter
    EVALTEST("(filter (lambda (x) (= (% x 2) 0)) [1 2 3 4 5 6 7 8 9 10])",
             L(5, scamint(2), scamint(4), scamint(6), scamint(8), scamint(10)));
    EVALTEST_ERR("(filter (lambda (x y) (and x y)) [1 2 3 4 5 ])");
    // range
    EVALTEST("(range -1 3)", L(4, scamint(-1), scamint(0), scamint(1), scamint(2)));
    EVALTEST_ERR("(range 0 -1)");
    EVALTEST_ERR("(range 1.0 3.0)");

    /*** STRING FUNCTIONS ***/
    EVALTEST("(upper \"humble\")", scamstr("HUMBLE"));
    EVALTEST("(lower \"HUMBLE\")", scamstr("humble"));
    EVALTEST("(isupper \"WHAT HATH GOD WROUGHT?\")", scambool(1));
    EVALTEST("(isupper \"What HATH GOD WROUGHT?\")", scambool(0));
    EVALTEST("(islower \"WHAT HATH GOD WROUGHT?\")", scambool(0));
    EVALTEST("(islower \"What HATH GOD WROUGHT?\")", scambool(0));
    EVALTEST("(islower \"what hath god wrought?\")", scambool(1));
    EVALTEST("(trim \"      a b       \")", scamstr("a b"));
    EVALTEST("(split \" a b c  d\")", L(4, scamstr("a"), scamstr("b"), scamstr("c"), scamstr("d")));

    /*** MATH FUNCTIONS ***/
    // ceil
    EVALTEST("(ceil 10.7)", scamint(11));
    EVALTEST("(ceil -10.7)", scamint(-10));
    // floor
    EVALTEST("(floor 10.7)", scamint(10));
    EVALTEST("(floor -10.7)", scamint(-11));
    // divmod
    EVALTEST("(divmod 10 3)", L(2, scamint(3), scamint(1)));
    EVALTEST_ERR("(divmod 10.0 3)");
    EVALTEST_ERR("(divmod 10 3.0)");
    // abs
    EVALTEST("(abs -10)", scamint(10));
    EVALTEST("(abs -10.0)", scamdec(10.0));
    EVALTEST("(abs 27.8)", scamdec(27.8));
    // sqrt
    EVALTEST("(sqrt 81)", scamdec(9.0));
    // pow
    EVALTEST("(pow 2 8)", scamint(256));
    EVALTEST("(pow 2.0 8)", scamdec(256.0));

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

void parsetest_err(char* line, int line_no) {
    scamval* v = parse_str(line);
    if (v->type != SCAM_ERR) {
        printf("Failed parse example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ERROR\n");
        printf("Got:\n  ");
        scamval_println(v);
        printf("\n");
    }
    gc_unset_root(v);
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
