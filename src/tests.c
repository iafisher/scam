#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"
#include "scamval.h"

void parsetest(char* line, const ScamVal* answer, int line_no);
void parsetest_err(char* line, int line_no);

void evaltest(char* line, const ScamVal* answer, ScamEnv* env, int line_no);
void evaltest_err(char* line, ScamEnv* env, int line_no);

int main() {
    #define PARSETEST(line, answer) parsetest(line, (ScamVal*)answer, __LINE__);
    #define PARSETEST_ERR(line) parsetest_err(line, __LINE__);
    #define EVALTEST(line, answer) evaltest(line, (ScamVal*)answer, env, __LINE__);
    #define EVALTEST_ERR(line) evaltest_err(line, env, __LINE__);
    #define EVALDEF(line) EVALTEST(line, ScamNull_new());
    #define S (ScamVal*)ScamExpr_from
    #define L (ScamVal*)ScamList_from
    #define D (ScamVal*)ScamDict_from
    puts("\n=== PARSER TESTS ===");
    puts("(you should see no failures)\n");

    /*** ATOMS ***/
    /* Numbers and booleans */
    PARSETEST("0", ScamInt_new(0));
    PARSETEST("-0", ScamInt_new(0));
    PARSETEST("174", ScamInt_new(174));
    PARSETEST("-78.3", ScamDec_new(-78.3));
    PARSETEST("0xff67", ScamInt_new(0xff67));
    PARSETEST("-0xff67", ScamInt_new(-0xff67));
    //PARSETEST_ERR("012"); // octal literals are not supported
    //PARSETEST_ERR("0x10g");
    //PARSETEST_ERR("0x-57");
    PARSETEST("true", ScamBool_new(true));
    PARSETEST("false", ScamBool_new(false));
    /* Strings and symbols */
    PARSETEST("matador", ScamSym_new("matador"));
    PARSETEST("true0", ScamSym_new("true0"));
    PARSETEST("false0", ScamSym_new("false0"));
    PARSETEST("\"matador\"", ScamStr_new("matador"));
    /* Expressions and lists */
    PARSETEST("(+ 1 1)", S(3, ScamSym_new("+"), ScamInt_new(1), ScamInt_new(1)));
    PARSETEST("[1 2 3]", S(4, ScamSym_new("list"), ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    PARSETEST("{1:\"one\"}", S(2, ScamSym_new("dict"),
                                  S(3, ScamSym_new("list"), ScamInt_new(1), ScamStr_new("one"))));
    /* Invalid expressions */
    PARSETEST_ERR("(+ (define x 10) 3)");

    puts("\n=== EVALUATOR TESTS ===");
    puts("(you should see two failed (+ 1 1) == 3 tests)\n");
    ScamEnv* env = ScamEnv_builtins();

    /*** DEFINE ***/
    EVALTEST("(define x 42) x", ScamInt_new(42));
    EVALTEST("(define x 666) x", ScamInt_new(666));
    /* Test shadowing a global variable in a local scope. */
    EVALTEST("(define (redefine-x) (define x 13) x)  (redefine-x)", ScamInt_new(13));
    /* Test shadowing a global variable with a function parameter. */
    EVALTEST("(define (x-as-parameter x) x)  (x-as-parameter 17)", ScamInt_new(17));
    /* Test that values are immutable. */
    EVALTEST("(define items [1 2 3]) (tail items)", L(2, ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("items", L(3, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    /* Test bad defines. */
    EVALTEST_ERR("(define 1 1)");
    EVALTEST_ERR("(define (1) 1)");
    EVALTEST_ERR("(define (foo 10) 10)");
    EVALTEST_ERR("(define x)");
    EVALTEST_ERR("(define (foo x))");

    /*** RECURSION ***/
    EVALTEST("(define (countdown x) (if (= x 0) x (countdown (- x 1))))", ScamNull_new());
    EVALTEST("(countdown 10)", ScamInt_new(0));
    /* A somewhat involved power function. */
    EVALDEF("(define (square x) (* x x))");
    EVALDEF("(define (power b n) (define (even? x) (= (% x 2) 0)) (if (= n 0) 1 (if (even? n) (square (power b (// n 2))) (* b (power b (- n 1))))))");
    EVALTEST("(power 287 0)", ScamInt_new(1));
    EVALTEST("(power 2 8)", ScamInt_new(256));
    EVALTEST("(power 17 8)", ScamInt_new(6975757441));

    /*** CLOSURES ***/
    EVALDEF("(define (make-fun x) (lambda (y) (+ x y)))");
    EVALTEST("((make-fun 10) 32)", ScamInt_new(42));
    /* A doubly nested closure. */
    EVALDEF("(define (make-fun1 x) (lambda (y) (lambda (z) (+ x y z))))");
    EVALTEST("(((make-fun1 1) 2) 3)", ScamInt_new(6));
    /* Another type of closure. */
    EVALDEF("(define (make-fun2) (define (double x) (* x 2)) double)");
    EVALDEF("(define foo (make-fun2))");
    EVALTEST("(foo 9)", ScamInt_new(18));
    /* Variables from the closure shouldn't exist in the global scope. */
    EVALTEST_ERR("double");

    /*** LAMBDA ***/
    EVALTEST("((lambda (x y) (+ x y)) 20 22)", ScamInt_new(42));
    EVALTEST("((lambda () (* 21 2)))", ScamInt_new(42));
    EVALTEST_ERR("((lambda (x y) (+ x y)) 20)");
    EVALTEST_ERR("((lambda (x y) (+ x y)) 20 21 22)");
    /* Parameters must be valid symbols. */
    EVALTEST_ERR("(lambda (10) (* 10 2))");
    EVALTEST_ERR("(lambda (x 10 y) (* x y))");
    /* Lambda expressions must have a body. */
    EVALTEST_ERR("(lambda (x))");

    /*** LIST and DICTIONARY LITERALS ***/
    EVALTEST("[(* 2 2) (* 3 3) (* 4 4)]", L(3, ScamInt_new(4), ScamInt_new(9), ScamInt_new(16)));
    EVALTEST("{1:\"one\"}", D(1, L(2, ScamInt_new(1), ScamStr_new("one"))));

    /*** ARITHMETIC FUNCTIONS ***/
    /* Addition */
    EVALTEST("(+ 7 -10 936 -14)", ScamInt_new(7 - 10 +936 - 14));
    EVALTEST("(+ 7.0 -10 936 -14)", ScamDec_new(7 - 10 + 936 - 14));
    /* Negation and subtraction */
    EVALTEST("(- 34.6)", ScamDec_new(-34.6));
    EVALTEST("(- 347 80 -2 17)", ScamInt_new(347 - 80 + 2 - 17));
    EVALTEST("(- 347.0 80 -2 17)", ScamDec_new(347 - 80 + 2 - 17));
    /* Multiplication */
    EVALTEST("(* 9 9 -437)", ScamInt_new(9 * 9 * -437));
    EVALTEST("(* 3.5 6.79 2.3)", ScamDec_new(3.5 * 6.79 * 2.3));
    /* Floating-point division */
    EVALTEST("(/ 10 3)", ScamDec_new(10 / 3.0));
    EVALTEST("(/ 3.7 8.91 2.3)", ScamDec_new((3.7 / 8.91) / 2.3));
    EVALTEST_ERR("(/ 1 0)");
    EVALTEST_ERR("(/ 10 7 0 4)");
    /* Floor division */
    EVALTEST("(// 10 3)", ScamInt_new(3));
    EVALTEST("(// -81 3 9)", ScamInt_new((-81 / 3) / 9));
    EVALTEST_ERR("(// 9 3.0)");
    EVALTEST_ERR("(// 1 0)");
    EVALTEST_ERR("(// 10 7 4 0)");
    /* Remainder */
    EVALTEST("(% 10 3)", ScamInt_new(1));
    EVALTEST("(% -76 4 18)", ScamInt_new((-76 % 4) % 18));
    EVALTEST_ERR("(% 9.0 3)");
    EVALTEST_ERR("(% 1 0)");
    EVALTEST_ERR("(% 10 0 7 4)");

    /*** BOOLEAN OPERATORS ***/
    EVALTEST("(and true true)", ScamBool_new(true));
    EVALTEST("(and true false)", ScamBool_new(false));
    EVALTEST("(and false true)", ScamBool_new(false));
    EVALTEST("(and false false)", ScamBool_new(false));
    EVALTEST("(and true true true true false)", ScamBool_new(false));
    EVALTEST("(or true true)", ScamBool_new(true));
    EVALTEST("(or true false)", ScamBool_new(true));
    EVALTEST("(or false true)", ScamBool_new(true));
    EVALTEST("(or false false)", ScamBool_new(false));
    EVALTEST("(not true)", ScamBool_new(false));
    EVALTEST("(not false)", ScamBool_new(true));
    /* Test short-circuiting. */
    EVALTEST("(and false (begin (/ 10 0) true))", ScamBool_new(false));
    EVALTEST_ERR("(and (begin (/ 10 0) true) false)");
    EVALTEST("(or true (begin (/ 10 0) true))", ScamBool_new(true));
    EVALTEST_ERR("(or (begin (/ 10 0) false) true)");

    /*** COMPARISON AND EQUALITY ***/
    /* Numeric equality */
    EVALTEST("(= 1 1)", ScamBool_new(true));
    EVALTEST("(= -81 -81.0)", ScamBool_new(true));
    EVALTEST("(= 1 0.99999)", ScamBool_new(false));
    EVALTEST("(= 1 \"1\")", ScamBool_new(false));
    /* String equality */
    EVALTEST("(=  \"money\"  \"money\")", ScamBool_new(true));
    EVALTEST("(=  \"lucre\"  \" lucre \")", ScamBool_new(false));
    EVALTEST("(=  \"\"  \"\")", ScamBool_new(true));
    /* Boolean equality */
    EVALTEST("(= true true)", ScamBool_new(true));
    EVALTEST("(= false false)", ScamBool_new(true));
    EVALTEST("(= true false)", ScamBool_new(false));
    EVALTEST("(= false true)", ScamBool_new(false));
    /* List equality */
    EVALTEST("(= [] [])", ScamBool_new(true));
    EVALTEST("(= [\"S\" [\"NP\" \"VP\"]] [\"S\" [\"VP\" \"NP\"]])", ScamBool_new(false));
    EVALTEST("(= [1 2 3 4 5] [1 2 3 4 5])", ScamBool_new(true));

    /*** LIST FUNCTIONS ***/
    EVALTEST("[1 2 3 4 5]", L(5, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3), ScamInt_new(4), ScamInt_new(5)));
    EVALTEST("[[\"inception\"]]", L(1, L(1, ScamStr_new("inception"))));
    /* empty? */
    EVALTEST("(empty? [])", ScamBool_new(true));
    EVALTEST("(empty? [[]])", ScamBool_new(false));
    /* len */
    EVALTEST("(len [])", ScamInt_new(0));
    EVALTEST("(len [1 2 3])", ScamInt_new(3));
    EVALTEST("(len [1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24])", ScamInt_new(24));
    /* head */
    EVALTEST("(head [1 2 3])", ScamInt_new(1));
    EVALTEST_ERR("(head [])");
    /* tail */
    EVALTEST("(tail [1 2 3])", L(2, ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("(tail [])", ScamList_new());
    /* last */
    EVALTEST("(last [1 2 3])", ScamInt_new(3));
    EVALTEST_ERR("(last [])");
    /* prepend */
    EVALTEST("(prepend 0 [1 2 3])", L(4, ScamInt_new(0), ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("(prepend 0 [])", L(1, ScamInt_new(0)));
    /* append */
    EVALTEST("(append [1 2 3] 4)", L(4, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3), ScamInt_new(4)));
    EVALTEST("(append [] 0)", L(1, ScamInt_new(0)));
    /* concat */
    EVALTEST("(concat [1 2] [3 4])", L(4, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3), ScamInt_new(4)));
    EVALTEST("(concat [1 2 3] [])", L(3, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("(concat [] [1 2 3])", L(3, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("(concat [] [])", ScamList_new());
    EVALTEST_ERR("(concat [] \"\")");
    /* get */
    EVALTEST("(get [\"I\" \"met\" \"a\" \"traveller\" \"from\" \"an\" \"antique\" \"land\"] 2)",
             ScamStr_new("a"));
    EVALTEST("(get [\"Fort Sumter\" \"Antietam\" \"Gettysburg\"] 2)", ScamStr_new("Gettysburg"));
    EVALTEST_ERR("(get [1] 1)");
    EVALTEST_ERR("(get [1] -1)");
    EVALTEST_ERR("(get [] 0)");
    /* slice */
    EVALTEST("(slice [1 2 3 4 5 6 7 8 9] 3 6)", L(3, ScamInt_new(4), ScamInt_new(5), ScamInt_new(6)));
    EVALTEST("(slice [1 2 3] 0 3)", L(3, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("(slice [1 2 3] 0 0)", ScamList_new());
    EVALTEST_ERR("(slice [1 2 3] 0 4)");
    EVALTEST_ERR("(slice [1 2 3] -1 3)");
    EVALTEST_ERR("(slice [1 2 3] 2 1)");
    /* take */
    EVALTEST("(take [1 2 3 4 5 6 7] 3)", L(3, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("(take [] 0)", ScamList_new());
    EVALTEST("(take [1 2 3] 3)", L(3, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    EVALTEST_ERR("(take [1 2 3] 4)");
    /* drop */
    EVALTEST("(drop [1 2 3 4 5 6 7] 3)", L(4, ScamInt_new(4), ScamInt_new(5), ScamInt_new(6), ScamInt_new(7)));
    EVALTEST("(drop [] 0)", ScamList_new());
    EVALTEST("(drop [1 2 3] 3)", ScamList_new());
    EVALTEST_ERR("(drop [1 2 3] 4)");
    /* insert */
    EVALTEST("(insert [\"one\" \"small\" \"step\" \"for\" \"man\"] 4 \"a\")",
             L(6, ScamStr_new("one"), ScamStr_new("small"), ScamStr_new("step"), ScamStr_new("for"), ScamStr_new("a"),
                  ScamStr_new("man")));
    EVALTEST("(insert [1 2 3] 0 0)", L(4, ScamInt_new(0), ScamInt_new(1), ScamInt_new(2), ScamInt_new(3)));
    EVALTEST("(insert [1 2 3] 3 4)", L(4, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3), ScamInt_new(4)));
    EVALTEST("(insert [] 0 \"first\")", L(1, ScamStr_new("first")));
    /* find */
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Germanic\")", ScamInt_new(0));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Slavic\")", ScamInt_new(1));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Romance\")", ScamInt_new(2));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"Indo-Iranian\")", ScamBool_new(false));
    EVALTEST("(find [\"Germanic\" \"Slavic\" \"Romance\"] \"romance\")", ScamBool_new(false));
    EVALTEST("(find [\"duplicate\" \"different\" \"duplicate\"] \"duplicate\")", ScamInt_new(0));
    /* rfind */
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Germanic\")", ScamInt_new(0));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Slavic\")", ScamInt_new(1));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Romance\")", ScamInt_new(2));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"Indo-Iranian\")", ScamBool_new(false));
    EVALTEST("(rfind [\"Germanic\" \"Slavic\" \"Romance\"] \"romance\")", ScamBool_new(false));
    EVALTEST("(rfind [\"duplicate\" \"different\" \"duplicate\"] \"duplicate\")", ScamInt_new(2));
    /* sort */
    EVALTEST("(sort [5 4 3 2 1])",
             L(5, ScamInt_new(1), ScamInt_new(2), ScamInt_new(3), ScamInt_new(4), ScamInt_new(5)));
    /* map */
    EVALTEST("(map (lambda (x) (* x 2)) [1 2 3 4 5])",
             L(5, ScamInt_new(2), ScamInt_new(4), ScamInt_new(6), ScamInt_new(8), ScamInt_new(10)));
    EVALTEST_ERR("(map (lambda (x y) (+ x y)) [1 2 3 4 5 ])");
    /* filter */
    EVALTEST("(filter (lambda (x) (= (% x 2) 0)) [1 2 3 4 5 6 7 8 9 10])",
             L(5, ScamInt_new(2), ScamInt_new(4), ScamInt_new(6), ScamInt_new(8), ScamInt_new(10)));
    EVALTEST_ERR("(filter (lambda (x y) (and x y)) [1 2 3 4 5 ])");
    /* range */
    EVALTEST("(range -1 3)", L(4, ScamInt_new(-1), ScamInt_new(0), ScamInt_new(1), ScamInt_new(2)));
    EVALTEST_ERR("(range 0 -1)");
    EVALTEST_ERR("(range 1.0 3.0)");

    /*** STRING FUNCTIONS ***/
    EVALTEST("(upper \"humble\")", ScamStr_new("HUMBLE"));
    EVALTEST("(lower \"HUMBLE\")", ScamStr_new("humble"));
    /* isupper and islower */
    EVALTEST("(isupper \"WHAT HATH GOD WROUGHT?\")", ScamBool_new(true));
    EVALTEST("(isupper \"What HATH GOD WROUGHT?\")", ScamBool_new(false));
    EVALTEST("(isupper \"...\")", ScamBool_new(false));
    EVALTEST("(islower \"WHAT HATH GOD WROUGHT?\")", ScamBool_new(false));
    EVALTEST("(islower \"What HATH GOD WROUGHT?\")", ScamBool_new(false));
    EVALTEST("(islower \"what hath god wrought?\")", ScamBool_new(true));
    EVALTEST("(islower \"...\")", ScamBool_new(false));
    EVALTEST("(trim \"      a b       \")", ScamStr_new("a b"));
    EVALTEST("(split \" a b c  d\")", L(4, ScamStr_new("a"), ScamStr_new("b"), ScamStr_new("c"), ScamStr_new("d")));

    /*** MATH FUNCTIONS ***/
    /* ceil */
    EVALTEST("(ceil 10.7)", ScamInt_new(11));
    EVALTEST("(ceil -10.7)", ScamInt_new(-10));
    /* floor */
    EVALTEST("(floor 10.7)", ScamInt_new(10));
    EVALTEST("(floor -10.7)", ScamInt_new(-11));
    /* divmod */
    EVALTEST("(divmod 10 3)", L(2, ScamInt_new(3), ScamInt_new(1)));
    EVALTEST_ERR("(divmod 10.0 3)");
    EVALTEST_ERR("(divmod 10 3.0)");
    /* abs */
    EVALTEST("(abs -10)", ScamInt_new(10));
    EVALTEST("(abs -10.0)", ScamDec_new(10.0));
    EVALTEST("(abs 27.8)", ScamDec_new(27.8));
    /* sqrt */
    EVALTEST("(sqrt 81)", ScamDec_new(9.0));
    /* pow */
    EVALTEST("(pow 2 8)", ScamInt_new(256));
    EVALTEST("(pow 2.0 8)", ScamDec_new(256.0));

    /*** IO FUNCTIONS ***/
    EVALDEF("(define fp (open \"resources/foo.txt\" \"r\"))");
    EVALTEST("(port-good? fp)", ScamBool_new(true));
    EVALTEST("(readline fp)", ScamStr_new("Lorem ipsum\n"));
    EVALTEST("(port-good? fp)", ScamBool_new(true));
    EVALTEST_ERR("(readline fp)");
    EVALTEST("(port-good? fp)", ScamBool_new(false));
    EVALDEF("(close fp)");

    /*** INTENTIONAL FAIL ***/
    EVALTEST("(+ 1 1)", ScamInt_new(3));
    EVALTEST_ERR("(+ 1 1)");
    gc_close();
    return 0;
}

void parsetest(char* line, const ScamVal* answer, int line_no) {
    ScamSeq* v = parse_str(line);
    ScamVal* modified_answer = (ScamVal*)ScamExpr_from(2, ScamSym_new("begin"), answer);
    if (!ScamVal_eq((ScamVal*)v, modified_answer)) {
        printf("Failed parse example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ");
        ScamVal_println(modified_answer);
        printf("Got:\n  ");
        ScamVal_println((ScamVal*)v);
        printf("\n");
    }
}

void parsetest_err(char* line, int line_no) {
    ScamSeq* v = parse_str(line);
    if (v->type != SCAM_ERR) {
        printf("Failed parse example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ERROR\n");
        printf("Got:\n  ");
        ScamVal_println((ScamVal*)v);
        printf("\n");
    }
    gc_unset_root((ScamVal*)v);
}

void evaltest(char* line, const ScamVal* answer, ScamEnv* env, int line_no) {
    ScamVal* v = eval_str(line, env);
    if (!ScamVal_eq(v, answer)) {
        printf("Failed example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ");
        ScamVal_println(answer);
        printf("Got:\n  ");
        ScamVal_println(v);
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

void evaltest_err(char* line, ScamEnv* env, int line_no) {
    ScamVal* v = eval_str(line, env);
    if (v->type != SCAM_ERR) {
        printf("Failed example, line %d in %s:\n", line_no, __FILE__);
        printf("  %s\n", line);
        printf("Expected:\n  ERROR\n");
        printf("Got:\n  ");
        ScamVal_println(v);
        printf("\n");
    }
    gc_unset_root(v);
}
