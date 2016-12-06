#include <stdarg.h>
#include "parse.h"

void parsetest(char* line, int n, ...) {
    scamval* ast = parse_line(line);
    if (ast->type != SCAM_CODE) {
        printf("Failed parse test \"%s\": expected SCAM_CODE object\n", line);
        scamval_free(ast);
        return;
    }
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        int type_we_want = va_arg(args, int);
        int type_we_got = scamval_get(ast, i)->type;
        if (type_we_want != type_we_got) {
            printf("Failed parse test \"%s\" on element %d; ", line, i);
            printf("got %s, expected %s\n", scamval_type_name(type_we_got),
                                            scamval_type_name(type_we_want));
            break;
        }
    }
    va_end(args);
    scamval_free(ast);
    printf("Passed parse test \"%s\"\n", line);
}

void parsetest_lit(char* line, int type_we_want) {
    scamval* v = parse_line(line);
    if (v->type == type_we_want) {
        printf("Passed parse test \"%s\"\n", line);
    } else {
        printf("Failed parse test \"%s\" ", line);
        printf("got %s, expected %s\n", scamval_type_name(v->type),
                                        scamval_type_name(type_we_want));
    }
    scamval_free(v);
}

void parse_tests() {
    parsetest_lit("-103", SCAM_INT);
    parsetest_lit("-103.7", SCAM_DEC);
    parsetest_lit("{1 2 3}", SCAM_QUOTE);
    parsetest_lit("hello", SCAM_SYM);
    parsetest_lit("\"hello\"", SCAM_STR);
    parsetest("(+ 1 1)", 3, SCAM_SYM, SCAM_INT, SCAM_INT);
    parsetest("(+ (* 9 2) 1)", 3, SCAM_SYM, SCAM_CODE, SCAM_INT);
    parsetest("((lambda (x y) (+ x y)) 5 5.0)", 3, SCAM_CODE, SCAM_INT, SCAM_DEC);
}
