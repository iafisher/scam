#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"
#include "scamval.h"


#define E (ScamVal*)ScamExpr_from
#define S ScamSym_new
#define I ScamInt_new


void benchmark(ScamVal* ast, unsigned int reps, ScamEnv* env, const char* test_name, FILE* fp);


int main() {
    char fname[100];
    time_t t = time(NULL);
    struct tm* tptr = gmtime(&t);
    strftime(fname, 100, "profile/%Y-%m-%d-%H:%M.txt", tptr);
    FILE* fp = fopen(fname, "w");
    if (!fp) {
        printf("Error: unable to open file %s\n", fname);
        return 1;
    }

    ScamEnv* env = ScamEnv_builtins();
    /* FUNCTION APPLICATION */
    eval_str("(define (f x) (* x 2))", env);
    /* (f 100) */
    benchmark(E(2, S("f"), I(100)), 100000, env, "Function application", fp);

    /* ARRAY APPEND */
    eval_str("(define items [])", env);
    /* (append items 0) */
    benchmark(E(3, S("append"), S("items"), I(0)), 100000, env, "Array append", fp);

    /* DICTIONARY INSERTION */
    eval_str("(define dct {}", env);
    clock_t begin = clock();
    for (int i = 0; i < 10000; i++) {
        /* (bind dct i i) */
        ScamVal* ast = E(4, S("bind"), S("dct"), I(i), I(i));
        eval(ast, env);
        gc_unset_root(ast);
    }
    clock_t end = clock();
    double this = (end - begin + 0.0) / CLOCKS_PER_SEC;
    fprintf(fp, "Dictionary insertion: %f seconds, 10000 reps\n", this);

    /* DICTIONARY LOOKUP */
    /* (get dct -1) */
    benchmark(E(3, S("get"), S("dct"), I(-1)), 10000, env, "Dictionary lookup", fp);

    fclose(fp);
    return 0;
}


void benchmark(ScamVal* ast, unsigned int reps, ScamEnv* env, const char* test_name, FILE* fp) {
    clock_t begin = clock();
    for (size_t i = 0; i < reps; i++) {
        ScamVal* ast_copy = gc_copy_ScamVal(ast);
        eval(ast_copy, env);
        gc_unset_root(ast_copy);
    }
    clock_t end = clock();
    double this = (end - begin + 0.0) / CLOCKS_PER_SEC;
    fprintf(fp, "%s: %f seconds, %d reps\n", test_name, this, reps);
}
