#include <stdarg.h>
#include "parse.h"
#include "scamtypes.h"

void parsetest(char* line, int n, ...);
void parsetest_lit(char* line, int type_we_want);

void parse_tests() {
    printf("Running parse tests\n");
    parsetest_lit("-103", SCAM_INT);
    parsetest_lit("-103.7", SCAM_DEC);
    parsetest_lit("hello", SCAM_SYM);
    parsetest_lit("\"hello\"", SCAM_STR);
    parsetest("(+ 1 1)", 3, SCAM_SYM, SCAM_INT, SCAM_INT);
    parsetest("(+ (* 9 2) 1)", 3, SCAM_SYM, SCAM_SEXPR, SCAM_INT);
    parsetest("((lambda (x y) (+ x y)) 5 5.0)", 3, SCAM_SEXPR, SCAM_INT, SCAM_DEC);
}

void parsetest(char* line, int n, ...) {
    scamval* ast = parse_str(line);
    if (ast->type != SCAM_SEXPR) {
        printf("Failed parse test \"%s\": expected SCAM_SEXPR object\n", line);
        scamval_free(ast);
        return;
    }
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        int req_type = va_arg(args, int);
        int given_type = scamseq_get(ast, i)->type;
        if (req_type != given_type) {
            printf("Failed parse test \"%s\" on element %d; ", line, i);
            printf("got %s, expected %s\n", scamtype_debug_name(given_type),
                                            scamtype_debug_name(req_type));
            break;
        }
    }
    va_end(args);
    scamval_free(ast);
}

void parsetest_lit(char* line, int type_we_want) {
    scamval* v = parse_str(line);
    if (v->type != type_we_want) {
        printf("Failed parse test \"%s\" ", line);
        printf("got %s, expected %s\n", scamtype_debug_name(v->type),
                                        scamtype_debug_name(type_we_want));
    }
    scamval_free(v);
}
