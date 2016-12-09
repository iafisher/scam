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
        return scamerr_type(name, i, type_we_got, type_we_want); \
    } \
}

#define TYPE_CHECK_UNARY(name, arglist, type_we_want) { \
    size_t n = scamval_len(arglist); \
    if (n == 1) { \
        TYPE_CHECK_POS(name, arglist, 0, type_we_want); \
    } else { \
        return scamerr_arity(name, n, 1); \
    } \
}

#define TYPECHECK_ARGS(name, arglist, n, ...) { \
    scamval* check_result = typecheck_args(name, arglist, n, ##__VA_ARGS__); \
    if (check_result) { \
        return check_result; \
    } \
}

#define TYPECHECK_ALL(name, arglist, pred) { \
    scamval* check_result = typecheck_all(name, arglist, pred); \
    if (check_result) { \
        return check_result; \
    } \
}

#define COUNT_ARGS(name, arglist, expected) { \
    size_t got = scamval_len(arglist); \
    if (got != expected) { \
        return scamerr_arity(name, got, expected); \
    } \
}

#define COUNT_MIN_ARGS(name, arglist, expected) { \
    size_t got = scamval_len(arglist); \
    if (got < expected) { \
        return scamerr_min_arity(name, got, expected); \
    } \
}

// Assert that no element of the arglist is equal to zero, unless only the 
// first element is zero and the arglist contains only two arguments
#define ASSERT_NO_ZEROS(arglist) { \
    if (scamval_len(arglist) > 2) { \
        scamval* first = scamval_get(arglist, 0); \
        if ((first->type == SCAM_INT && first->vals.n == 0) || \
            (first->type == SCAM_DEC && first->vals.d == 0.0)) { \
            return scamerr("cannot divide by zero"); \
        } \
    } \
    for (int i = 1; i < scamval_len(arglist); i++) { \
        scamval* v = scamval_get(arglist, i); \
        if ((v->type == SCAM_INT && v->vals.n == 0) || \
            (v->type == SCAM_DEC && v->vals.d == 0.0)) { \
            return scamerr("cannot divide by zero"); \
        } \
    } \
}

typedef int type_pred(int);
int is_scamlist(int type) { return type == SCAM_LIST; }
int is_scamint(int type) { return type == SCAM_INT; }
int is_scamnum(int type) { return type == SCAM_INT || type == SCAM_DEC; }

scamval* typecheck_args(char* name, scamval* arglist, size_t arity, ...) {
    COUNT_ARGS(name, arglist, arity);
    va_list vlist;
    va_start(vlist, arity);
    for (int i = 0; i < scamval_len(arglist); i++) {
        type_pred* this_pred = va_arg(vlist, type_pred*);
        if (!this_pred(scamval_get(arglist, i)->type))
            return scamerr_type2(name, i, scamval_get(arglist, i)->type);
    }
    return NULL;
}

scamval* typecheck_all(char* name, scamval* arglist, type_pred pred) {
    for (int i = 0; i < scamval_len(arglist); i++) {
        int this_type = scamval_get(arglist, i)->type;
        if (!pred(this_type)) {
            return scamerr_type2(name, i, this_type);
        }
    }
    return NULL;
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


typedef long long (int_arith_func)(long long, long long);
scamval* generic_int_arith(char* name, scamval* arglist, int_arith_func op) {
    COUNT_MIN_ARGS(name, arglist, 2);
    TYPECHECK_ALL(name, arglist, is_scamint);
    /*
    int type = arglist_type(arglist);
    if (type != SCAM_INT) {
        return scamerr("'%s' passed non-integer argument", name);
    }
    */
    scamval* first = scamval_get(arglist, 0);
    long long sum = first->vals.n;
    for (int i = 1; i < scamval_len(arglist); i++) {
        sum = op(sum, scamval_get(arglist, i)->vals.n);
    }
    return scamint(sum);
}

typedef double (arith_func)(double, double);
scamval* generic_mixed_arith(char* name, scamval* arglist, arith_func op, 
                             int coerce_to_double) {
    COUNT_MIN_ARGS(name, arglist, 2);
    TYPECHECK_ALL(name, arglist, is_scamnum);
    /*
    int type = arglist_type(arglist);
    if (type != SCAM_INT && type != SCAM_DEC) {
        return scamerr("'%s' passed non-numeric argument", name);
    }
    */
    scamval* first = scamval_get(arglist, 0);
    double sum = first->type == SCAM_INT ? first->vals.n : first->vals.d;
    int seen_double = 0;
    for (int i = 1; i < scamval_len(arglist); i++) {
        scamval* v = scamval_get(arglist, i);
        if (v->type == SCAM_INT) {
            sum = op(sum, v->vals.n);
        } else {
            sum = op(sum, v->vals.d);
            seen_double = 1;
        }
    }
    if (seen_double || coerce_to_double) {
        return scamdec(sum);
    } else {
        return scamint(sum);
    }
}

double arith_add(double x, double y) { return x + y; }
scamval* builtin_add(scamval* arglist) {
    return generic_mixed_arith("+", arglist, arith_add, 0);
}

scamval* builtin_negate(scamval* arglist) {
    int type = arglist_type(arglist);
    if (type == SCAM_INT) {
        long long n = scamval_get(arglist, 0)->vals.n;
        return scamint(-1 * n);
    } else if (type == SCAM_DEC) {
        double d = scamval_get(arglist, 0)->vals.d;
        return scamdec(-1 * d);
    } else {
        return scamerr("'-' passed non-numeric argument");
    }
}

double arith_sub(double x, double y) { return x - y; }
scamval* builtin_sub(scamval* arglist) {
    if (scamval_len(arglist) == 1) {
        return builtin_negate(arglist);
    } else {
        return generic_mixed_arith("-", arglist, arith_sub, 0);
    }
}

double arith_mult(double x, double y) { return x * y; }
scamval* builtin_mult(scamval* arglist) {
    return generic_mixed_arith("*", arglist, arith_mult, 0);
}

long long arith_rem(long long x, long long y) { return x % y; }
scamval* builtin_rem(scamval* arglist) {
    ASSERT_NO_ZEROS(arglist);
    return generic_int_arith("%", arglist, arith_rem);
}

double arith_real_div(double x, double y) { return x / y; }
scamval* builtin_real_div(scamval* arglist) {
    ASSERT_NO_ZEROS(arglist);
    return generic_mixed_arith("/", arglist, arith_real_div, 1);
}

long long arith_floor_div(long long x, long long y) { return x / y; }
scamval* builtin_floor_div(scamval* arglist) {
    ASSERT_NO_ZEROS(arglist);
    return generic_int_arith("//", arglist, arith_floor_div);
}

scamval* builtin_len(scamval* arglist) {
    //TYPE_CHECK_UNARY("len", arglist, SCAM_LIST);
    TYPECHECK_ARGS("len", arglist, 1, is_scamlist);
    return scamint(scamval_len(scamval_get(arglist, 0)));
}

scamval* builtin_empty(scamval* arglist) {
    TYPE_CHECK_UNARY("len", arglist, SCAM_LIST);
    return scambool(scamval_len(scamval_get(arglist, 0)) == 0);
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
        return scamerr("attempted array access out of range");
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
        return scamerr("cannot take head of empty list");
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
    COUNT_MIN_ARGS("concat", arglist, 1);
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
            return scamerr("'concat' passed non-sequence argument");
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
            return scamint(i);
        }
    }
    return scambool(0);
}

scamval* builtin_rfind(scamval* arglist) {
    COUNT_ARGS("rfind", arglist, 2);
    TYPE_CHECK_POS("rfind", arglist, 0, SCAM_LIST);
    scamval* list_arg = scamval_get(arglist, 0);
    scamval* datum = scamval_get(arglist, 1);
    for (int i = scamval_len(list_arg) - 1; i >= 0; i--) {
        if (scamval_eq(scamval_get(list_arg, i), datum)) {
            return scamint(i);
        }
    }
    return scambool(0);
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
    return scamnull();
}

scamval* builtin_println(scamval* arglist) {
    COUNT_ARGS("println", arglist, 1);
    scamval* arg = scamval_get(arglist, 0);
    if (arg->type != SCAM_STR) {
        scamval_println(arg);
    } else {
        printf("%s\n", arg->vals.s);
    }
    return scamnull();
}

scamval* builtin_input(scamval* arglist) {
    COUNT_ARGS("input", arglist, 0);
    char* s = NULL;
    size_t s_len = 0;
    size_t last = getline(&s, &s_len, stdin);
    // remove the trailing newline
    if (last > 0 && s[last - 1] == '\n') {
        s[last - 1] = '\0';
    }
    scamval* ret = scamstr(s);
    free(s);
    return ret;
}

scamval* builtin_begin(scamval* arglist) {
    COUNT_MIN_ARGS("begin", arglist, 1);
    return scamval_pop(arglist, scamval_len(arglist) - 1);
}

scamval* builtin_eq(scamval* arglist) {
    COUNT_ARGS("=", arglist, 2);
    scamval* left = scamval_get(arglist, 0);
    scamval* right = scamval_get(arglist, 1);
    return scambool(scamval_eq(left, right));
}

typedef int (comp_func)(int, int);
scamval* generic_comparison(char* name, scamval* arglist, comp_func op) {
    COUNT_ARGS(name, arglist, 2);
    TYPE_CHECK_POS(name, arglist, 0, SCAM_INT);
    TYPE_CHECK_POS(name, arglist, 1, SCAM_INT);
    scamval* left = scamval_get(arglist, 0);
    scamval* right = scamval_get(arglist, 1);
    return scambool(op(left->vals.n, right->vals.n));
}

int comp_gt(int x, int y) { return x > y; }
scamval* builtin_gt(scamval* arglist) {
    return generic_comparison(">", arglist, comp_gt);
}

int comp_lt(int x, int y) { return x < y; }
scamval* builtin_lt(scamval* arglist) {
    return generic_comparison("<", arglist, comp_lt);
}


int comp_gte(int x, int y) { return x >= y; }
scamval* builtin_gte(scamval* arglist) {
    return generic_comparison(">=", arglist, comp_gte);
}

int comp_lte(int x, int y) { return x <= y; }
scamval* builtin_lte(scamval* arglist) {
    return generic_comparison("<=", arglist, comp_lte);
}

scamval* builtin_not(scamval* arglist) {
    TYPE_CHECK_UNARY("not", arglist, SCAM_BOOL);
    return scambool(!(scamval_get(arglist, 0)->vals.n));
}

void add_builtin(scamenv* env, char* sym, scambuiltin_t bltin) {
    scamenv_bind(env, scamsym(sym), scambuiltin(bltin));
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
    add_builtin(env, "input", builtin_input);
}
