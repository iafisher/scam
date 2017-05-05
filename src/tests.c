#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"
#include "scamval.h"

void evaltest(char* line, const scamval* answer, scamval* env, int line_no);
void evaltest_err(char* line, scamval* env, int line_no);

int main(int argc, char* argv[]) {
    #define EVALTEST(line, answer) evaltest(line, answer, env, __LINE__);
    #define EVALTEST_ERR(line) evaltest_err(line, env, __LINE__);
    puts("\n=== EVALUATOR TESTS ===");
    puts("(you should see a single failed (+ 1 1) == 3 test)\n");
    scamval* env = scamdict_builtins();
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
    //EVALTEST("[1 2 3 4 5]", scam
    // designed to fail
    EVALTEST("(+ 1 1)", scamint(3));
    gc_close();
    return 0;
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
