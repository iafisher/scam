#include <stdio.h>
#include <string.h>
#include "builtins.h"

#define TYPE_CHECK_ONE(name, arglist, i, type_we_want) { \
    int type_we_got = scamval_get(arglist, i)->type; \
    if (type_we_got != type_we_want) { \
        scamval_free(arglist); \
        return scamval_err("type mismatch in arg %d of '%s': got %s, expected %s", i, name, scamval_type_name(type_we_got), scamval_type_name(type_we_want)); \
    } \
}

#define COUNT_ARGS(name, arglist, req_n) { \
    size_t num_of_args = scamval_len(arglist); \
    if (num_of_args != req_n) { \
        scamval_free(arglist); \
        return scamval_err("'%s' expected %d argument(s), got %d", \
                           name, req_n, num_of_args); \
    } \
}

#define COUNT_ARGS_AT_LEAST(name, arglist, req_n) { \
    size_t num_of_args = scamval_len(arglist); \
    if (num_of_args < req_n) { \
        scamval_free(arglist); \
        return scamval_err("'%s' expected at least %d argument(s), got %d", \
                           name, req_n, num_of_args); \
    } \
}

// BUILTIN_MIXED_ARITHMETIC, but without the error checking
#define BUILTIN_MIXED_ARITHMETIC_CORE(arglist, op) { \
    scamval* first = scamval_get(arglist, 0); \
    double sum = first->type == SCAM_INT ? first->vals.n : first->vals.d; \
    for (int i = 1; i < scamval_len(arglist); i++) { \
        scamval* v = scamval_get(arglist, i); \
        if (v->type == SCAM_INT) { \
            sum op##= scamval_get(arglist, i)->vals.n; \
        } else { \
            sum op##= scamval_get(arglist, i)->vals.d; \
        } \
    } \
    scamval_free(arglist); \
    return scamval_dec(sum); \
}

// BUILTIN_INT_ARITHMETIC, but without the error checking
#define BUILTIN_INT_ARITHMETIC_CORE(arglist, op) { \
    scamval* first = scamval_get(arglist, 0); \
    long long sum = first->vals.n; \
    for (int i = 1; i < scamval_len(arglist); i++) { \
        sum op##= scamval_get(arglist, i)->vals.n; \
    } \
    scamval_free(arglist); \
    return scamval_int(sum); \
}

// Perform the integer on the given arglist, returning an error if any of the 
// operands are not integers
#define BUILTIN_INT_ARITHMETIC(name, arglist, op) { \
    COUNT_ARGS_AT_LEAST(name, arglist, 1); \
    int type = arglist_type(arglist); \
    if (type != SCAM_INT) { \
        scamval_free(arglist); \
        return scamval_err("'%s' passed non-integer argument", name); \
    } \
    BUILTIN_INT_ARITHMETIC_CORE(arglist, op); \
}

// Perform the operation on the given arglist, coercing each argument into the
// decimal type even if all arguments are integers.
#define BUILTIN_DEC_ARITHMETIC(name, arglist, op) { \
    COUNT_ARGS_AT_LEAST(name, arglist, 1); \
    int type = arglist_type(arglist); \
    if (type != SCAM_INT && type != SCAM_DEC) { \
        scamval_free(arglist); \
        return scamval_err("'%s' passed non-numeric argument", name); \
    } \
    BUILTIN_MIXED_ARITHMETIC_CORE(arglist, op); \
}

// Perform the operation on the given arglist, coercing each argument into the
// decimal type only if not all arguments are integers.
#define BUILTIN_MIXED_ARITHMETIC(name, arglist, op) { \
    COUNT_ARGS_AT_LEAST(name, arglist, 1); \
    int type = arglist_type(arglist); \
    if (type == SCAM_INT) { \
        BUILTIN_INT_ARITHMETIC_CORE(arglist, op); \
    } else if (type == SCAM_DEC) { \
        BUILTIN_MIXED_ARITHMETIC_CORE(arglist, op); \
    } else { \
        scamval_free(arglist); \
        return scamval_err("'%s' passed non-numeric argument", name); \
    } \
}

// Return SCAM_INT if all args are ints, SCAM_DEC if at least one is a decimal,
// or the first non-numeric type if one is found
int arglist_type(scamval* arglist) {
    int found_decimal = 0;
    for (int i = 0; i < scamval_len(arglist); i++) {
        int type = scamval_get(arglist, i)->type;
        if (type == SCAM_DEC) {
            found_decimal = 1;
        } else if (type != SCAM_INT) {
            return type;
        }
    }
    return found_decimal ? SCAM_DEC : SCAM_INT;
}

scamval* builtin_add(scamval* arglist) {
    BUILTIN_MIXED_ARITHMETIC("+", arglist, +);
}

scamval* builtin_negate(scamval* arglist) {
    int type = arglist_type(arglist);
    if (type == SCAM_INT) {
        long long n = scamval_get(arglist, 0)->vals.n;
        scamval_free(arglist);
        return scamval_int(-1 * n);
    } else if (type == SCAM_DEC) {
        double d = scamval_get(arglist, 0)->vals.d;
        scamval_free(arglist);
        return scamval_dec(-1 * d);
    } else {
        scamval_free(arglist);
        return scamval_err("'-' passed non-numeric argument");
    }
}

scamval* builtin_sub(scamval* arglist) {
    if (scamval_len(arglist) == 1) {
        return builtin_negate(arglist);
    } else {
        BUILTIN_MIXED_ARITHMETIC("-", arglist, -);
    }
}

scamval* builtin_mult(scamval* arglist) {
    BUILTIN_MIXED_ARITHMETIC("*", arglist, *);
}

scamval* builtin_rem(scamval* arglist) {
    BUILTIN_INT_ARITHMETIC("%", arglist, %);
}

scamval* builtin_real_div(scamval* arglist) {
    BUILTIN_DEC_ARITHMETIC("/", arglist, /);
}

scamval* builtin_floor_div(scamval* arglist) {
    BUILTIN_INT_ARITHMETIC("//", arglist, /);
}

scamval* builtin_len(scamval* arglist) {
    COUNT_ARGS("len", arglist, 1);
    TYPE_CHECK_ONE("len", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    scamval* ret = scamval_int(scamval_len(list_arg));
    scamval_free(arglist);
    return ret;
}

scamval* builtin_head(scamval* arglist) {
    COUNT_ARGS("head", arglist, 1);
    TYPE_CHECK_ONE("head", arglist, 0, SCAM_LIST);
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
    COUNT_ARGS("tail", arglist, 1);
    TYPE_CHECK_ONE("tail", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_pop(arglist, 0);
    scamval_free(scamval_pop(list_arg, 0));
    scamval_free(arglist);
    return list_arg;
}

scamval* builtin_last(scamval* arglist) {
    COUNT_ARGS("last", arglist, 1);
    TYPE_CHECK_ONE("last", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    scamval* ret = scamval_pop(list_arg, scamval_len(list_arg) - 1);
    scamval_free(arglist);
    return ret;
}

scamval* builtin_init(scamval* arglist) {
    COUNT_ARGS("init", arglist, 1);
    TYPE_CHECK_ONE("init", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_pop(arglist, 0);
    scamval_free(scamval_pop(list_arg, scamval_len(list_arg) - 1));
    scamval_free(arglist);
    return list_arg;
}

scamval* builtin_pop(scamval* arglist) {
    COUNT_ARGS("pop", arglist, 2);
    TYPE_CHECK_ONE("pop", arglist, 0, SCAM_LIST);
    TYPE_CHECK_ONE("pop", arglist, 1, SCAM_INT);
    scamval* list_arg = scamval_get(arglist, 0);
    size_t i = scamval_get(arglist, 1)->vals.n;
    scamval* ret = scamval_pop(list_arg, i);
    scamval_free(arglist);
    return ret;
}

scamval* builtin_print(scamval* arglist) {
    COUNT_ARGS("print", arglist, 1);
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
    COUNT_ARGS("println", arglist, 1);
    scamval* arg = scamval_get(arglist, 0);
    if (arg->type != SCAM_STR) {
        scamval_println(arg);
    } else {
        printf("%s\n", arg->vals.s);
    }
    scamval_free(arglist);
    return scamval_null();
}

scamval* builtin_begin(scamval* arglist) {
    COUNT_ARGS_AT_LEAST("begin", arglist, 1);
    scamval* last_arg = scamval_pop(arglist, scamval_len(arglist) - 1);
    scamval_free(arglist);
    return last_arg;
}

scamval* builtin_eq(scamval* arglist) {
    COUNT_ARGS("=", arglist, 2);
    TYPE_CHECK_ONE("=", arglist, 0, SCAM_INT);
    TYPE_CHECK_ONE("=", arglist, 1, SCAM_INT);
    scamval* left = scamval_get(arglist, 0);
    scamval* right = scamval_get(arglist, 1);
    int ret = (left->vals.n == right->vals.n);
    scamval_free(arglist);
    return scamval_bool(ret);
}

void add_builtin(scamenv* env, char* sym, scambuiltin bltin) {
    scamval* sym_val = scamval_sym(sym);
    scamval* bltin_val = scamval_builtin(bltin);
    scamenv_bind(env, sym_val, bltin_val);
    scamval_free(sym_val); scamval_free(bltin_val);
}

void register_builtins(scamenv* env) {
    add_builtin(env, "begin", builtin_begin);
    add_builtin(env, "+", builtin_add);
    add_builtin(env, "-", builtin_sub);
    add_builtin(env, "*", builtin_mult);
    add_builtin(env, "/", builtin_real_div);
    add_builtin(env, "//", builtin_floor_div);
    add_builtin(env, "%", builtin_rem);
    add_builtin(env, "=", builtin_eq);
    add_builtin(env, "len", builtin_len);
    add_builtin(env, "head", builtin_head);
    add_builtin(env, "tail", builtin_tail);
    add_builtin(env, "last", builtin_last);
    add_builtin(env, "init", builtin_init);
    add_builtin(env, "pop", builtin_pop);
    add_builtin(env, "print", builtin_print);
    add_builtin(env, "println", builtin_println);
}
