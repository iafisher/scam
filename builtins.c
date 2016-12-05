#include <stdio.h>
#include <string.h>
#include "builtins.h"

#define TYPE_CHECK_ALL(args, req_type) { \
    for (int i = 1; i < args->count; i++) { \
        if (args->root[i]->type != req_type) \
            return scamval_err("type mismatch"); \
    } \
}

#define TYPE_CHECK_ONE(args, i, req_type) { \
    if (i < 0 || i >= args->count || args->root[i] != req_type) { \
        return scamval_err("type mismatch"); \
    } \
}

#define COUNT_ARGS(args, req_n) { \
    if (args->count - 1 != req_n) { \
        return scamval_err("wrong number of arguments"); \
    } \
}

scamval* builtin_add(array* args) {
    TYPE_CHECK_ALL(args, SCAM_INT);
    long long sum = 0;
    for (int i = 1; i < args->count; i++) {
        sum += args->root[i]->vals.n;
    }
    return scamval_int(sum);
}

scamval* builtin_sub(array* args) {
    TYPE_CHECK_ALL(args, SCAM_INT);
    if (args->count == 2) {
        return scamval_int(-1 * args->root[1]->vals.n);
    } else {
        long long sum = args->root[1]->vals.n;
        for (int i = 2; i < args->count; i++) {
            sum -= args->root[i]->vals.n;
        }
        return scamval_int(sum);
    }
}

scamval* builtin_mult(array* args) {
    TYPE_CHECK_ALL(args, SCAM_INT);
    long long product = 1;
    for (int i = 1; i < args->count; i++) {
        product *= args->root[i]->vals.n;
    }
    return scamval_int(product);
}

scamval* builtin_head(array* args) {
    COUNT_ARGS(args, 1);
    TYPE_CHECK_ALL(args, SCAM_LIST);
    if (scamval_len(args->root[1]) > 0) {
        return scamval_copy(scamval_get(args->root[1], 0));
    } else {
        return scamval_err("cannot take head of empty list");
    }
}

scamval* builtin_tail(array* args) {
    COUNT_ARGS(args, 1);
    TYPE_CHECK_ALL(args, SCAM_LIST);
    scamval* ret = scamval_list();
    for (int i = 1; i < scamval_len(args->root[1]); i++) {
        scamval_append(ret, scamval_copy(scamval_get(args->root[1], i)));
    }
    return ret;
}

scamval* builtin_print(array* args) {
    COUNT_ARGS(args, 1);
    if (args->root[1]->type != SCAM_STR) {
        scamval_print(args->root[1]);
    } else {
        printf("%s", args->root[1]->vals.s);
    }
    return scamval_null();
}

scamval* builtin_println(array* args) {
    COUNT_ARGS(args, 1);
    if (args->root[1]->type != SCAM_STR) {
        scamval_println(args->root[1]);
    } else {
        printf("%s\n", args->root[1]->vals.s);
    }
    return scamval_null();
}

void add_builtin(scamenv* env, char* sym, scambuiltin bltin) {
    scamenv_bind(env, scamval_sym(sym), scamval_builtin(bltin));
}

void register_builtins(scamenv* env) {
    add_builtin(env, "+", builtin_add);
    add_builtin(env, "-", builtin_sub);
    add_builtin(env, "*", builtin_mult);
    add_builtin(env, "head", builtin_head);
    add_builtin(env, "tail", builtin_tail);
    add_builtin(env, "print", builtin_print);
    add_builtin(env, "println", builtin_println);
}
