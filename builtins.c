#include <stdio.h>
#include <string.h>
#include "builtins.h"

#define TYPE_CHECK_ALL(arglist, req_type) { \
    for (int i = 0; i < scamval_len(arglist); i++) { \
        if (scamval_get(arglist, i)->type != req_type) { \
            scamval_free(arglist); \
            return scamval_err("type mismatch"); \
        } \
    } \
}

#define TYPE_CHECK_ONE(arglist, i, req_type) { \
    if (i < 0 || i >= scamval_len(arglist) || \
        scamval_get(arglist, i)->type != req_type) { \
        scamval_free(arglist); \
        return scamval_err("type mismatch"); \
    } \
}

#define COUNT_ARGS(arglist, req_n) { \
    if (scamval_len(arglist) != req_n) { \
        scamval_free(arglist); \
        return scamval_err("wrong number of arguments"); \
    } \
}

scamval* builtin_add(scamval* arglist) {
    TYPE_CHECK_ALL(arglist, SCAM_INT);
    long long sum = 0;
    for (int i = 0; i < scamval_len(arglist); i++) {
        sum += scamval_get(arglist, i)->vals.n;
    }
    scamval_free(arglist);
    return scamval_int(sum);
}

scamval* builtin_sub(scamval* arglist) {
    TYPE_CHECK_ALL(arglist, SCAM_INT);
    if (scamval_len(arglist) == 1) {
        // unary negation
        return scamval_int(-1 * scamval_get(arglist, 0)->vals.n);
    } else {
        // variadic subtraction
        long long sum = scamval_get(arglist, 0)->vals.n;
        for (int i = 1; i < scamval_len(arglist); i++) {
            sum -= scamval_get(arglist, i)->vals.n;
        }
        scamval_free(arglist);
        return scamval_int(sum);
    }
}

scamval* builtin_mult(scamval* arglist) {
    TYPE_CHECK_ALL(arglist, SCAM_INT);
    long long product = 1;
    for (int i = 0; i < scamval_len(arglist); i++) {
        product *= scamval_get(arglist, i)->vals.n;
    }
    scamval_free(arglist);
    return scamval_int(product);
}

scamval* builtin_head(scamval* arglist) {
    COUNT_ARGS(arglist, 1);
    TYPE_CHECK_ALL(arglist, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    if (scamval_len(list_arg) > 0) {
        scamval* ret = scamval_pop(list_arg, 0);
        scamval_free(arglist);
        return ret;
    } else {
        scamval_free(arglist);
        return scamval_err("cannot take head of empty list");
    }
}

scamval* builtin_tail(scamval* arglist) {
    COUNT_ARGS(arglist, 1);
    TYPE_CHECK_ALL(arglist, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    scamval_pop(list_arg, 0);
    return list_arg;
}

scamval* builtin_pop(scamval* arglist) {
    COUNT_ARGS(arglist, 2);
    TYPE_CHECK_ONE(arglist, 0, SCAM_LIST);
    TYPE_CHECK_ONE(arglist, 1, SCAM_INT);
    scamval* list_arg = scamval_get(arglist, 0);
    size_t i = scamval_get(arglist, 1)->vals.n;
    scamval* ret = scamval_pop(list_arg, i);
    scamval_free(arglist);
    return ret;
}

scamval* builtin_print(scamval* arglist) {
    COUNT_ARGS(arglist, 1);
    scamval* arg = scamval_get(arglist, 0);
    if (arg->type != SCAM_STR) {
        scamval_print(arg);
    } else {
        printf("%s", arg->vals.s);
    }
    scamval_free(arglist);
    return scamval_null();
}

scamval* builtin_println(scamval* arglist) {
    COUNT_ARGS(arglist, 1);
    scamval* arg = scamval_get(arglist, 0);
    if (arg->type != SCAM_STR) {
        scamval_println(arg);
    } else {
        printf("%s\n", arg->vals.s);
    }
    scamval_free(arglist);
    return scamval_null();
}

void add_builtin(scamenv* env, char* sym, scambuiltin bltin) {
    scamval* sym_val = scamval_sym(sym);
    scamval* bltin_val = scamval_builtin(bltin);
    scamenv_bind(env, sym_val, bltin_val);
    scamval_free(sym_val); scamval_free(bltin_val);
}

void register_builtins(scamenv* env) {
    add_builtin(env, "+", builtin_add);
    add_builtin(env, "-", builtin_sub);
    add_builtin(env, "*", builtin_mult);
    add_builtin(env, "head", builtin_head);
    add_builtin(env, "tail", builtin_tail);
    add_builtin(env, "pop", builtin_pop);
    add_builtin(env, "print", builtin_print);
    add_builtin(env, "println", builtin_println);
}
