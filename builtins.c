#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "eval.h"

#define TYPE_CHECK_POS(name, arglist, i, type_we_want) { \
    int type_we_got = scamval_get(arglist, i)->type; \
    if (type_we_got != type_we_want) { \
        return scamval_err("type mismatch in arg %d of '%s': got %s, expected %s", i + 1, name, scamval_type_name(type_we_got), scamval_type_name(type_we_want)); \
    } \
}

#define TYPE_CHECK_UNARY(name, arglist, type_we_want) { \
    size_t n = scamval_len(arglist); \
    if (n == 1) { \
        TYPE_CHECK_POS(name, arglist, 0, type_we_want); \
    } else { \
        return scamval_err("'%s' expected 1 argument, got %d", name, n); \
    } \
}

#define COUNT_ARGS(name, arglist, req_n) { \
    size_t num_of_args = scamval_len(arglist); \
    if (num_of_args != req_n) { \
        return scamval_err("'%s' expected %d argument(s), got %d", \
                           name, req_n, num_of_args); \
    } \
}

#define COUNT_ARGS_AT_LEAST(name, arglist, req_n) { \
    size_t num_of_args = scamval_len(arglist); \
    if (num_of_args < req_n) { \
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
    return scamval_dec(sum); \
}

// BUILTIN_INT_ARITHMETIC, but without the error checking
#define BUILTIN_INT_ARITHMETIC_CORE(arglist, op) { \
    scamval* first = scamval_get(arglist, 0); \
    long long sum = first->vals.n; \
    for (int i = 1; i < scamval_len(arglist); i++) { \
        sum op##= scamval_get(arglist, i)->vals.n; \
    } \
    return scamval_int(sum); \
}

// Perform the integer on the given arglist, returning an error if any of the 
// operands are not integers
#define BUILTIN_INT_ARITHMETIC(name, arglist, op) { \
    COUNT_ARGS_AT_LEAST(name, arglist, 1); \
    int type = arglist_type(arglist); \
    if (type != SCAM_INT) { \
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
        return scamval_err("'%s' passed non-numeric argument", name); \
    } \
}

#define BUILTIN_COMPARISON(name, arglist, op) { \
    COUNT_ARGS(name, arglist, 2); \
    TYPE_CHECK_POS(name, arglist, 0, SCAM_INT); \
    TYPE_CHECK_POS(name, arglist, 1, SCAM_INT); \
    scamval* left = scamval_get(arglist, 0); \
    scamval* right = scamval_get(arglist, 1); \
    return scamval_bool(left->vals.n op right->vals.n); \
}

typedef int type_pred(int);
int typecheck_arglist(scamval* arglist, size_t arity, ...) {
    size_t num_of_args = scamval_len(arglist);
    if (num_of_args != arity)
        return 0;
    va_list vlist;
    va_start(vlist, arity);
    for (int i = 0; i < num_of_args; i++) {
        type_pred* this_pred = va_arg(vlist, type_pred);
        if (!this_pred(scamval_get(arglist, i)->type))
            return 0;
    }
    return 1;
}

int is_scam_list(int type) { return type == SCAM_LIST; }

scamval* builtin_max(scamval* left, scamval* right) {
    if (left->vals.n >= right->vals.n) {
        scamval_free(right);
        return left;
    } else {
        scamval_free(left);
        return right;
    }
}

// Think about how to generate this as a macro from builtin_max
scamval* builtin_max_wrapper(scamval* arglist) {
    //TYPE_CHECK_BINARY("max", arglist, SCAM_INT, SCAM_INT);
    scamval* arg1 = scamval_pop(arglist, 0);
    scamval* arg2 = scamval_pop(arglist, 0);
    scamval* ret = builtin_max(arg1, arg2);
    scamval_free(arglist);
    return ret;
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
        return scamval_int(-1 * n);
    } else if (type == SCAM_DEC) {
        double d = scamval_get(arglist, 0)->vals.d;
        return scamval_dec(-1 * d);
    } else {
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
    TYPE_CHECK_UNARY("len", arglist, SCAM_LIST);
    return scamval_int(scamval_len(scamval_get(arglist, 0)));
}

scamval* builtin_empty(scamval* arglist) {
    TYPE_CHECK_UNARY("len", arglist, SCAM_LIST);
    return scamval_bool(scamval_len(scamval_get(arglist, 0)) == 0);
}

scamval* builtin_get(scamval* arglist) {
    COUNT_ARGS("get", arglist, 2);
    TYPE_CHECK_POS("get", arglist, 0, SCAM_LIST);
    TYPE_CHECK_POS("get", arglist, 1, SCAM_INT);
    scamval* list_arg = scamval_get(arglist, 0);
    size_t i = scamval_get(arglist, 1)->vals.n;
    return scamval_pop(list_arg, i);
}

scamval* builtin_slice(scamval* arglist) {
    COUNT_ARGS("slice", arglist, 3);
    TYPE_CHECK_POS("slice", arglist, 0, SCAM_LIST);
    TYPE_CHECK_POS("slice", arglist, 1, SCAM_INT);
    TYPE_CHECK_POS("slice", arglist, 2, SCAM_INT);
    size_t start = scamval_get(arglist, 1)->vals.n;
    size_t end = scamval_get(arglist, 2)->vals.n;
    scamval* list_arg = scamval_pop(arglist, 0);
    size_t n = scamval_len(list_arg);
    if (start < 0 || start >= n || end < 0 || end >= n || start > end) {
        scamval_free(list_arg);
        return scamval_err("attempted array access out of range");
    }
    for (int i = 0; i < n; i++) {
        if (i < start) {
            scamval_free(scamval_pop(list_arg, 0));
        } else if (i >= end) {
            scamval_free(scamval_pop(list_arg, scamval_len(list_arg) - 1));
        }
    }
    return list_arg;
}

scamval* builtin_take(scamval* arglist) {
    COUNT_ARGS("take", arglist, 2);
    TYPE_CHECK_POS("take", arglist, 0, SCAM_LIST);
    TYPE_CHECK_POS("take", arglist, 1, SCAM_INT);
    size_t start = scamval_get(arglist, 1)->vals.n;
    scamval* list_arg = scamval_pop(arglist, 0);
    size_t n = scamval_len(list_arg);
    for (int i = start; i < n; i++) {
        scamval_free(scamval_pop(list_arg, start));
    }
    return list_arg;
}

scamval* builtin_drop(scamval* arglist) {
    COUNT_ARGS("drop", arglist, 2);
    TYPE_CHECK_POS("drop", arglist, 0, SCAM_LIST);
    TYPE_CHECK_POS("drop", arglist, 1, SCAM_INT);
    size_t end = scamval_get(arglist, 1)->vals.n;
    scamval* list_arg = scamval_pop(arglist, 0);
    size_t n = scamval_len(list_arg);
    for (int i = 0; i < n; i++) {
        if (i < end) {
            scamval_free(scamval_pop(list_arg, 0));
        } else {
            break;
        }
    }
    return list_arg;
}

scamval* builtin_head(scamval* arglist) {
    TYPE_CHECK_UNARY("head", arglist, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    if (scamval_len(list_arg) > 0) {
        return scamval_pop(list_arg, 0);
    } else {
        return scamval_err("cannot take head of empty list");
    }
}

scamval* builtin_tail(scamval* arglist) {
    TYPE_CHECK_UNARY("tail", arglist, SCAM_LIST);
    scamval* list_arg = scamval_pop(arglist, 0);
    scamval_free(scamval_pop(list_arg, 0));
    return list_arg;
}

scamval* builtin_last(scamval* arglist) {
    TYPE_CHECK_UNARY("last", arglist, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    return scamval_pop(list_arg, scamval_len(list_arg) - 1);
}

scamval* builtin_init(scamval* arglist) {
    TYPE_CHECK_UNARY("init", arglist, SCAM_LIST);
    scamval* list_arg = scamval_pop(arglist, 0);
    scamval_free(scamval_pop(list_arg, scamval_len(list_arg) - 1));
    return list_arg;
}

scamval* builtin_append(scamval* arglist) {
    COUNT_ARGS("append", arglist, 2);
    TYPE_CHECK_POS("append", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_pop(arglist, 0);
    scamval* v = scamval_pop(arglist, 0);
    scamval_append(list_arg, v);
    return list_arg;
}

scamval* builtin_prepend(scamval* arglist) {
    COUNT_ARGS("append", arglist, 2);
    TYPE_CHECK_POS("append", arglist, 1, SCAM_LIST);
    scamval* v = scamval_pop(arglist, 0);
    scamval* list_arg = scamval_pop(arglist, 0);
    scamval_prepend(list_arg, v);
    return list_arg;
}

scamval* builtin_concat(scamval* arglist) {
    COUNT_ARGS_AT_LEAST("concat", arglist, 1);
    TYPE_CHECK_POS("concat", arglist, 0, SCAM_LIST);
    size_t n = scamval_len(arglist);
    scamval* first_arg = scamval_pop(arglist, 0);
    for (int i = 1; i < n; i++) {
        scamval* this_arg = scamval_get(arglist, 0);
        if (this_arg->type == first_arg->type) {
            size_t n2 = scamval_len(this_arg);
            for (int j = 0; j < n2; j++) {
                scamval_append(first_arg, scamval_pop(this_arg, 0));
            }
        } else {
            scamval_free(first_arg);
            return scamval_err("'concat' passed non-sequence argument");
        }
    }
    return first_arg;
}

scamval* builtin_find(scamval* arglist) {
    COUNT_ARGS("find", arglist, 2);
    TYPE_CHECK_POS("find", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    scamval* datum = scamval_get(arglist, 1);
    for (int i = 0; i < scamval_len(list_arg); i++) {
        if (scamval_eq(scamval_get(list_arg, i), datum)) {
            return scamval_int(i);
        }
    }
    return scamval_bool(0);
}

scamval* builtin_rfind(scamval* arglist) {
    COUNT_ARGS("rfind", arglist, 2);
    TYPE_CHECK_POS("rfind", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    scamval* datum = scamval_get(arglist, 1);
    for (int i = scamval_len(list_arg) - 1; i >= 0; i--) {
        if (scamval_eq(scamval_get(list_arg, i), datum)) {
            return scamval_int(i);
        }
    }
    return scamval_bool(0);
}

scamval* builtin_upper(scamval* arglist) {
    TYPE_CHECK_UNARY("upper", arglist, SCAM_STR);
    scamval* str_arg = scamval_pop(arglist, 0);
    for (int i = 0; i < strlen(str_arg->vals.s); i++) {
        str_arg->vals.s[i] = toupper(str_arg->vals.s[i]);
    }
    return str_arg;
}

scamval* builtin_lower(scamval* arglist) {
    TYPE_CHECK_UNARY("lower", arglist, SCAM_STR);
    scamval* str_arg = scamval_pop(arglist, 0);
    for (int i = 0; i < strlen(str_arg->vals.s); i++) {
        str_arg->vals.s[i] = tolower(str_arg->vals.s[i]);
    }
    return str_arg;
}

scamval* builtin_print(scamval* arglist) {
    COUNT_ARGS("print", arglist, 1);
    scamval* arg = scamval_get(arglist, 0);
    if (arg->type != SCAM_STR) {
        scamval_print(arg);
    } else {
        printf("%s", arg->vals.s);
    }
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
    return scamval_null();
}

scamval* builtin_begin(scamval* arglist) {
    COUNT_ARGS_AT_LEAST("begin", arglist, 1);
    return scamval_pop(arglist, scamval_len(arglist) - 1);
}

scamval* builtin_eq(scamval* arglist) {
    COUNT_ARGS("=", arglist, 2);
    scamval* left = scamval_get(arglist, 0);
    scamval* right = scamval_get(arglist, 1);
    return scamval_bool(scamval_eq(left, right));
}

scamval* builtin_gt(scamval* arglist) {
    BUILTIN_COMPARISON(">", arglist, >);
}

scamval* builtin_lt(scamval* arglist) {
    BUILTIN_COMPARISON("<", arglist, <);
}

scamval* builtin_gte(scamval* arglist) {
    BUILTIN_COMPARISON(">=", arglist, >=);
}

scamval* builtin_lte(scamval* arglist) {
    BUILTIN_COMPARISON("<=", arglist, <=);
}

scamval* builtin_not(scamval* arglist) {
    TYPE_CHECK_UNARY("not", arglist, SCAM_BOOL);
    return scamval_bool(!(scamval_get(arglist, 0)->vals.n));
}

void add_builtin(scamenv* env, char* sym, scambuiltin bltin) {
    scamenv_bind(env, scamval_sym(sym), scamval_builtin(bltin));
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
    add_builtin(env, ">", builtin_gt);
    add_builtin(env, "<", builtin_lt);
    add_builtin(env, ">=", builtin_gte);
    add_builtin(env, "<=", builtin_lte);
    add_builtin(env, "not", builtin_not);
    // sequence functions
    add_builtin(env, "len", builtin_len);
    add_builtin(env, "empty?", builtin_empty);
    add_builtin(env, "head", builtin_head);
    add_builtin(env, "tail", builtin_tail);
    add_builtin(env, "last", builtin_last);
    add_builtin(env, "init", builtin_init);
    add_builtin(env, "get", builtin_get);
    add_builtin(env, "slice", builtin_slice);
    add_builtin(env, "take", builtin_take);
    add_builtin(env, "drop", builtin_drop);
    add_builtin(env, "append", builtin_append);
    add_builtin(env, "prepend", builtin_prepend);
    add_builtin(env, "concat", builtin_concat);
    add_builtin(env, "find", builtin_find);
    add_builtin(env, "rfind", builtin_rfind);
    // string functions
    add_builtin(env, "upper", builtin_upper);
    add_builtin(env, "lower", builtin_lower);
    // IO functions
    add_builtin(env, "print", builtin_print);
    add_builtin(env, "println", builtin_println);
}
