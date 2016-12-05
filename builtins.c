#include "builtins.h"

#define TYPE_CHECK_ALL(args, req_type) { \
    for (int i = 1; i < args->count; i++) { \
        if (args->root[i]->type != req_type) \
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

void register_builtins(scamenv* env) {
    scamenv_bind(env, scamval_sym("+"), scamval_builtin(builtin_add));
    scamenv_bind(env, scamval_sym("-"), scamval_builtin(builtin_sub));
    scamenv_bind(env, scamval_sym("*"), scamval_builtin(builtin_mult));
    scamenv_bind(env, scamval_sym("head"), scamval_builtin(builtin_head));
    scamenv_bind(env, scamval_sym("tail"), scamval_builtin(builtin_tail));
}
