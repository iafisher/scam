#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"

#define TYPECHECK_ARGS(name, args, n, ...) { \
    scamval* check_result = typecheck_args(name, args, n, ##__VA_ARGS__); \
    if (check_result) { \
        return check_result; \
    } \
}

#define TYPECHECK_ALL(name, args, min, type) { \
    scamval* check_result = typecheck_all(name, args, min, type); \
    if (check_result) { \
        return check_result; \
    } \
}

// Assert that no element of the args is equal to zero, unless only the 
// first element is zero and the args contains only two arguments
#define ASSERT_NO_ZEROS(args) { \
    if (scamseq_len(args) > 2) { \
        scamval* first = scamseq_get(args, 0); \
        if ((first->type == SCAM_INT && scam_as_int(first) == 0) || \
            (first->type == SCAM_DEC && scam_as_dec(first) == 0.0)) { \
            return scamerr("cannot divide by zero"); \
        } \
    } \
    for (int i = 1; i < scamseq_len(args); i++) { \
        scamval* v = scamseq_get(args, i); \
        if ((v->type == SCAM_INT && scam_as_int(v) == 0) || \
            (v->type == SCAM_DEC && scam_as_dec(v) == 0.0)) { \
            return scamerr("cannot divide by zero"); \
        } \
    } \
}

scamval* typecheck_args(char* name, scamval* args, size_t arity, ...) {
    size_t n = scamseq_len(args);
    if (n != arity) {
        return scamerr_arity(name, n, arity);
    }
    va_list vlist;
    va_start(vlist, arity);
    for (int i = 0; i < scamseq_len(args); i++) {
        int type_we_need = va_arg(vlist, int);
        scamval* v = scamseq_get(args, i);
        if (!scamval_typecheck(v, type_we_need)) {
            return scamerr_type(name, i, v->type, type_we_need);
        }
    }
    return NULL;
}

scamval* typecheck_all(char* name, scamval* args, int min, int type_we_need) {
    size_t n = scamseq_len(args);
    if (n < min) {
        return scamerr_min_arity(name, n, min);
    }
    for (int i = 0; i < n; i++) {
        scamval* v = scamseq_get(args, i);
        if (!scamval_typecheck(v, type_we_need)) {
            return scamerr_type(name, i, v->type, type_we_need);
        }
    }
    return NULL;
}

typedef long long (int_arith_func)(long long, long long);
scamval* generic_int_arith(char* name, scamval* args, int_arith_func op) {
    TYPECHECK_ALL(name, args, 2, SCAM_INT);
    scamval* first = scamseq_get(args, 0);
    long long sum = scam_as_int(first);
    for (int i = 1; i < scamseq_len(args); i++) {
        sum = op(sum, scam_as_int(scamseq_get(args, i)));
    }
    return scamint(sum);
}

typedef double (arith_func)(double, double);
scamval* generic_mixed_arith(char* name, scamval* args, arith_func op, 
                             int coerce_to_double) {
    TYPECHECK_ALL(name, args, 2, SCAM_NUM);
    scamval* first = scamseq_get(args, 0);
    double sum = first->type == SCAM_INT ? scam_as_int(first) : 
                                           scam_as_dec(first);
    int seen_double = 0;
    for (int i = 1; i < scamseq_len(args); i++) {
        scamval* v = scamseq_get(args, i);
        if (v->type == SCAM_INT) {
            sum = op(sum, scam_as_int(v));
        } else {
            sum = op(sum, scam_as_dec(v));
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
scamval* builtin_add(scamval* args) {
    return generic_mixed_arith("+", args, arith_add, 0);
}

scamval* builtin_negate(scamval* args) {
    TYPECHECK_ARGS("-", args, 1, SCAM_NUM);
    scamval* v = scamseq_get(args, 0);
    if (v->type == SCAM_INT) {
        return scamint(-1 * scam_as_int(v));
    } else {
        return scamint(-1 * scam_as_dec(v));
    }
}

double arith_sub(double x, double y) { return x - y; }
scamval* builtin_sub(scamval* args) {
    if (scamseq_len(args) == 1) {
        return builtin_negate(args);
    } else {
        return generic_mixed_arith("-", args, arith_sub, 0);
    }
}

double arith_mult(double x, double y) { return x * y; }
scamval* builtin_mult(scamval* args) {
    return generic_mixed_arith("*", args, arith_mult, 0);
}

long long arith_rem(long long x, long long y) { return x % y; }
scamval* builtin_rem(scamval* args) {
    ASSERT_NO_ZEROS(args);
    return generic_int_arith("%", args, arith_rem);
}

double arith_real_div(double x, double y) { return x / y; }
scamval* builtin_real_div(scamval* args) {
    ASSERT_NO_ZEROS(args);
    return generic_mixed_arith("/", args, arith_real_div, 1);
}

long long arith_floor_div(long long x, long long y) { return x / y; }
scamval* builtin_floor_div(scamval* args) {
    ASSERT_NO_ZEROS(args);
    return generic_int_arith("//", args, arith_floor_div);
}

scamval* builtin_len(scamval* args) {
    TYPECHECK_ARGS("len", args, 1, SCAM_SEQ);
    scamval* arg = scamseq_get(args, 0);
    if (arg->type == SCAM_STR) {
        return scamint(scamstr_len(arg));
    } else {
        return scamint(scamseq_len(arg));
    } 
}

scamval* builtin_empty(scamval* args) {
    TYPECHECK_ARGS("empty?", args, 1, SCAM_SEQ);
    scamval* arg = scamseq_get(args, 0);
    if (arg->type == SCAM_STR) {
        return scambool(scamstr_len(arg) == 0);
    } else {
        return scambool(scamseq_len(arg) == 0);
    } 
}

scamval* builtin_list_get(scamval* args) {
    scamval* list_arg = scamseq_get(args, 0);
    size_t i = scam_as_int(scamseq_get(args, 1));
    if (i >= 0 && i < scamseq_len(list_arg)) {
        return gc_copy_scamval(scamseq_get(list_arg, i));
    } else {
        return scamerr("attempted sequence access out of range");
    }
}

scamval* builtin_str_get(scamval* args) {
    scamval* str_arg = scamseq_get(args, 0);
    size_t i = scam_as_int(scamseq_get(args, 1));
    if (i >= 0 && i < scamstr_len(str_arg)) {
        return scamstr_from_char(scamstr_get(str_arg, i));
    } else {
        return scamerr("attempted string access out of range");
    }
}

scamval* builtin_get(scamval* args) {
    TYPECHECK_ARGS("get", args, 2, SCAM_SEQ, SCAM_INT);
    int type = scamseq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_get(args);
    } else {
        return builtin_list_get(args);
    }
}

scamval* builtin_slice(scamval* args) {
    TYPECHECK_ARGS("slice", args, 3, SCAM_LIST, SCAM_INT, SCAM_INT);
    size_t start = scam_as_int(scamseq_get(args, 1));
    size_t end = scam_as_int(scamseq_get(args, 2));
    scamval* list_arg = scamseq_pop(args, 0);
    size_t n = scamseq_len(list_arg);
    if (start < 0 || start >= n || end < 0 || end >= n || start > end) {
        gc_unset_root(list_arg);
        return scamerr("attempted sequence access out of range");
    }
    for (int i = 0; i < n; i++) {
        if (i < start) {
            scamseq_delete(list_arg, 0);
        } else if (i >= end) {
            scamseq_delete(list_arg, scamseq_len(list_arg) - 1);
        }
    }
    return list_arg;
}

scamval* builtin_take(scamval* args) {
    TYPECHECK_ARGS("take", args, 2, SCAM_LIST, SCAM_INT);
    size_t start = scam_as_int(scamseq_get(args, 1));
    scamval* list_arg = scamseq_pop(args, 0);
    size_t n = scamseq_len(list_arg);
    for (int i = start; i < n; i++) {
        scamseq_delete(list_arg, start);
    }
    return list_arg;
}

scamval* builtin_drop(scamval* args) {
    TYPECHECK_ARGS("drop", args, 2, SCAM_LIST, SCAM_INT);
    size_t end = scam_as_int(scamseq_get(args, 1));
    scamval* list_arg = scamseq_pop(args, 0);
    size_t n = scamseq_len(list_arg);
    for (int i = 0; i < n; i++) {
        if (i < end) {
            scamseq_delete(list_arg, 0);
        } else {
            break;
        }
    }
    return list_arg;
}

scamval* builtin_list_head(scamval* args) {
    scamval* list_arg = scamseq_get(args, 0);
    if (scamseq_len(list_arg) > 0) {
        return gc_copy_scamval(scamseq_get(list_arg, 0));
    } else {
        return scamerr("cannot take head of empty list");
    }
}

scamval* builtin_str_head(scamval* args) {
    scamval* str_arg = scamseq_get(args, 0);
    if (scamstr_len(str_arg) > 0) {
        return scamstr_from_char(scamstr_get(str_arg, 0));
    } else {
        return scamerr("cannot take head of empty string");
    }
}

scamval* builtin_head(scamval* args) {
    TYPECHECK_ARGS("head", args, 1, SCAM_SEQ);
    int type = scamseq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_head(args);
    } else {
        return builtin_list_head(args);
    }
}

scamval* builtin_list_tail(scamval* args) {
    scamval* list_arg = scamseq_pop(args, 0);
    scamseq_delete(list_arg, 0);
    return list_arg;
}

scamval* builtin_str_tail(scamval* args) {
    scamval* str_arg = scamseq_pop(args, 0);
    scamstr_pop(str_arg, 0);
    return str_arg;
}

scamval* builtin_tail(scamval* args) {
    TYPECHECK_ARGS("tail", args, 1, SCAM_SEQ);
    int type = scamseq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_tail(args);
    } else {
        return builtin_list_tail(args);
    }
}

scamval* builtin_list_last(scamval* args) {
    scamval* list_arg = scamseq_get(args, 0);
    if (scamseq_len(list_arg) > 0) {
        return gc_copy_scamval(scamseq_get(list_arg, scamseq_len(list_arg)-1));
    } else {
        return scamerr("cannot take last of empty list");
    }
}

scamval* builtin_str_last(scamval* args) {
    scamval* str_arg = scamseq_get(args, 0);
    size_t n = scamstr_len(str_arg);
    if (n > 0) {
        return scamstr_from_char(scamstr_get(str_arg, n - 1));
    } else {
        return scamerr("cannot take last of empty string");
    }
}

scamval* builtin_last(scamval* args) {
    TYPECHECK_ARGS("last", args, 1, SCAM_SEQ);
    int type = scamseq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_last(args);
    } else {
        return builtin_list_last(args);
    } 
}

scamval* builtin_list_init(scamval* args) {
    scamval* list_arg = scamseq_pop(args, 0);
    scamseq_delete(list_arg, scamseq_len(list_arg) - 1);
    return list_arg;
}

scamval* builtin_str_init(scamval* args) {
    scamval* str_arg = scamseq_pop(args, 0);
    size_t n = scamstr_len(str_arg);
    scamstr_pop(str_arg, n - 1);
    return str_arg;
}

scamval* builtin_init(scamval* args) {
    TYPECHECK_ARGS("init", args, 1, SCAM_SEQ);
    int type = scamseq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_init(args);
    } else {
        return builtin_list_init(args);
    }
}

scamval* builtin_append(scamval* args) {
    TYPECHECK_ARGS("append", args, 2, SCAM_LIST, SCAM_ANY);
    scamval* list_arg = scamseq_pop(args, 0);
    scamval* v = scamseq_pop(args, 0);
    scamseq_append(list_arg, v);
    return list_arg;
}

scamval* builtin_prepend(scamval* args) {
    TYPECHECK_ARGS("prepend", args, 2, SCAM_ANY, SCAM_LIST);
    scamval* v = scamseq_pop(args, 0);
    scamval* list_arg = scamseq_pop(args, 0);
    scamseq_prepend(list_arg, v);
    return list_arg;
}

scamval* builtin_list_concat(scamval* args) {
    scamval* first_arg = scamseq_pop(args, 0);
    while (scamseq_len(args) > 0) {
        scamseq_concat(first_arg, scamseq_pop(args, 0));
    }
    return first_arg;
}

scamval* builtin_str_concat(scamval* args) {
    scamval* first_arg = scamseq_pop(args, 0);
    while (scamseq_len(args) > 0) {
        scamstr_concat(first_arg, scamseq_pop(args, 0));
    }
    return first_arg;
}

scamval* builtin_concat(scamval* args) {
    TYPECHECK_ALL("concat", args, 2, SCAM_SEQ);
    int narrowest_type = scamseq_narrowest_type(args);
    if (narrowest_type == SCAM_LIST) {
        return builtin_list_concat(args);
    } else if (narrowest_type == SCAM_STR) {
        return builtin_str_concat(args);
    } else {
        return scamerr("cannot concatenate lists and strings");
    }
}

scamval* builtin_find(scamval* args) {
    TYPECHECK_ARGS("find", args, 2, SCAM_LIST, SCAM_ANY);
    scamval* list_arg = scamseq_get(args, 0);
    scamval* datum = scamseq_get(args, 1);
    for (int i = 0; i < scamseq_len(list_arg); i++) {
        if (scamval_eq(scamseq_get(list_arg, i), datum)) {
            return scamint(i);
        }
    }
    return scambool(0);
}

scamval* builtin_rfind(scamval* args) {
    TYPECHECK_ARGS("rfind", args, 2, SCAM_LIST, SCAM_ANY);
    scamval* list_arg = scamseq_get(args, 0);
    scamval* datum = scamseq_get(args, 1);
    for (int i = scamseq_len(list_arg) - 1; i >= 0; i--) {
        if (scamval_eq(scamseq_get(list_arg, i), datum)) {
            return scamint(i);
        }
    }
    return scambool(0);
}

scamval* builtin_upper(scamval* args) {
    TYPECHECK_ARGS("upper", args, 1, SCAM_STR);
    scamval* str_arg = scamseq_pop(args, 0);
    scamstr_map(str_arg, toupper);
    return str_arg;
}

scamval* builtin_lower(scamval* args) {
    TYPECHECK_ARGS("lower", args, 1, SCAM_STR);
    scamval* str_arg = scamseq_pop(args, 0);
    scamstr_map(str_arg, tolower);
    return str_arg;
}

scamval* builtin_trim(scamval* args) {
    TYPECHECK_ARGS("trim", args, 1, SCAM_STR);
    scamval* str_arg = scamseq_pop(args, 0);
    size_t n = scamstr_len(str_arg);
    // remove left whitespace
    size_t left_ws = 0;
    while (isspace(scamstr_get(str_arg, left_ws)) && left_ws < n)
        left_ws++;
    if (left_ws > 0) {
        scamstr_remove(str_arg, 0, left_ws);
    }
    // remove right whitespace
    n -= left_ws;
    size_t right_ws = 0;
    while (isspace(scamstr_get(str_arg, n - (right_ws + 1))) && right_ws < n)
        right_ws++;
    scamstr_truncate(str_arg, n - right_ws);
    return str_arg;
}

scamval* builtin_split(scamval* args) {
    TYPECHECK_ARGS("split", args, 1, SCAM_STR);
    scamval* ret = scamlist();
    scamval* str_arg = scamseq_get(args, 0);
    int start = 0;
    int in_word = 0;
    for (int i = 0; i < scamstr_len(str_arg); i++) {
        if (!isspace(scamstr_get(str_arg, i))) {
            if (!in_word) {
                in_word = 1;
                start = i;
            }
        } else {
            if (in_word) {
                in_word = 0;
                scamseq_append(ret, scamstr_substr(str_arg, start, i));
            }
        }
    }
    // make sure to add the last word
    if (in_word) {
        scamseq_append(ret, scamstr_substr(str_arg, start, scamstr_len(str_arg)));
    }
    return ret;
}

scamval* builtin_print(scamval* args) {
    TYPECHECK_ARGS("print", args, 1, SCAM_ANY);
    scamval* arg = scamseq_get(args, 0);
    if (arg->type != SCAM_STR) {
        scamval_print(arg);
    } else {
        printf("%s", scam_as_str(arg));
    }
    return scamnull();
}

scamval* builtin_println(scamval* args) {
    TYPECHECK_ARGS("println", args, 1, SCAM_ANY);
    scamval* arg = scamseq_get(args, 0);
    if (arg->type != SCAM_STR) {
        scamval_println(arg);
    } else {
        printf("%s\n", scam_as_str(arg));
    }
    return scamnull();
}

scamval* builtin_open(scamval* args) {
    TYPECHECK_ARGS("open", args, 2, SCAM_STR, SCAM_STR);
    const char* fname = scam_as_str(scamseq_get(args, 0));
    const char* mode = scam_as_str(scamseq_get(args, 1));
    FILE* fp = fopen(fname, mode);
    return scamport(fp);
}

scamval* builtin_close(scamval* args) {
    TYPECHECK_ARGS("close", args, 1, SCAM_PORT);
    scamval* port_arg = scamseq_get(args, 0);
    if (scamport_status(port_arg) == SCAMPORT_OPEN) {
        fclose(scam_as_file(port_arg));
        scamport_set_status(port_arg, SCAMPORT_CLOSED);
        return scamnull();
    } else {
        return scamerr("port is already closed");
    }
}

scamval* builtin_port_good(scamval* args) {
    TYPECHECK_ARGS("port-good?", args, 1, SCAM_PORT);
    scamval* port_arg = scamseq_get(args, 0);
    if (scamport_status(port_arg) == SCAMPORT_OPEN) {
        FILE* fp = scam_as_file(port_arg);
        return scambool(!ferror(fp) && !feof(fp));
    } else {
        return scambool(0);
    }
}

scamval* builtin_readline(scamval* args) {
    if (scamseq_len(args) == 0) {
        return scamstr_read(stdin);
    } else {
        TYPECHECK_ARGS("readline", args, 1, SCAM_PORT);
        scamval* port_arg = scamseq_get(args, 0);
        if (scamport_status(port_arg) == SCAMPORT_OPEN) {
            return scamstr_read(scam_as_file(port_arg));
        } else {
            return scamerr("cannot read from closed port");
        }
    }
}

scamval* builtin_readchar(scamval* args) {
    TYPECHECK_ARGS("readchar", args, 1, SCAM_PORT);
    scamval* port_arg = scamseq_get(args, 0);
    if (scamport_status(port_arg) == SCAMPORT_OPEN) {
        char c = fgetc(port_arg->vals.port->fp);
        if (c != EOF) {
            return scamstr_from_char(c);
        } else {
            return scamerr_eof();
        }
    } else {
        return scamerr("cannot read from closed port");
    }
}

scamval* builtin_assert(scamval* args) {
    TYPECHECK_ARGS("assert", args, 1, SCAM_BOOL);
    scamval* cond = scamseq_get(args, 0);
    if (scam_as_bool(cond)) {
        return scambool(1);
    } else {
        return scamerr("failed assert at line %d, col %d", cond->line, 
                                                           cond->col);
    }
}

scamval* builtin_range(scamval* args) {
    TYPECHECK_ARGS("range", args, 2, SCAM_INT, SCAM_INT);
    scamval* lower = scamseq_get(args, 0);
    scamval* upper = scamseq_get(args, 1);
    if (scam_as_int(lower) <= scam_as_int(upper)) {
        int count = scam_as_int(lower);
        scamval* ret = scamlist();
        while (count < scam_as_int(upper)) {
            scamseq_append(ret, scamint(count));
            count++;
        }
        return ret;
    } else {
        return scamerr("lower bound must be less than or equal to upper bound "
                       "in function 'range'");
    }
}

int scamval_cmp(const void* a, const void* b) {
    const scamval* v1 = *(const scamval**)a;
    const scamval* v2 = *(const scamval**)b;
    if (scamval_gt(v1, v2) == 1) {
        return 1;
    } else if (scamval_eq(v1, v2) == 1) {
        return 0;
    } else {
        return -1;
    }
}

scamval* builtin_sort(scamval* args) {
    TYPECHECK_ARGS("sort", args, 1, SCAM_LIST);
    scamval* list_arg = scamseq_pop(args, 0);
    qsort(list_arg->vals.arr, scamseq_len(list_arg), 
          sizeof *list_arg->vals.arr, scamval_cmp);
    return list_arg;
}

scamval* builtin_map(scamval* args) {
    TYPECHECK_ARGS("map", args, 2, SCAM_FUNCTION, SCAM_LIST);
    scamval* fun = scamseq_pop(args, 0);
    scamval* list_arg = scamseq_pop(args, 0);
    for (size_t i = 0; i < scamseq_len(list_arg); i++) {
        scamval* v = scamseq_get(list_arg, i);
        scamval* arglist = scamsexpr_from_vals(1, v);
        scamseq_set(list_arg, i, eval_apply(fun, arglist));
        gc_unset_root(arglist);
    }
    gc_unset_root(fun);
    return list_arg;
}

scamval* builtin_filter(scamval* args) {
    TYPECHECK_ARGS("filter", args, 2, SCAM_FUNCTION, SCAM_LIST);
    scamval* fun = scamseq_pop(args, 0);
    scamval* list_arg = scamseq_pop(args, 0);
    for (size_t i = 0; i < scamseq_len(list_arg); i++) {
        scamval* v = scamseq_get(list_arg, i);
        scamval* arglist = scamsexpr_from_vals(1, v);
        scamval* cond = eval_apply(fun, arglist);
        gc_unset_root(arglist);
        if (cond->type == SCAM_BOOL) {
            if (!scam_as_bool(cond)) {
                scamseq_delete(list_arg, i);
            }
            gc_unset_root(cond);
        } else {
            gc_unset_root(cond);
            return scamerr("'filter' predicate should return boolean");
        }
    }
    gc_unset_root(fun);
    return list_arg;
}

scamval* builtin_id(scamval* args) {
    TYPECHECK_ARGS("id", args, 1, SCAM_ANY);
    return scamint((long long)scamseq_get(args, 0));
}

scamval* builtin_begin(scamval* args) {
    TYPECHECK_ALL("begin", args, 1, SCAM_ANY);
    return scamseq_pop(args, scamseq_len(args) - 1);
}

scamval* builtin_eq(scamval* args) {
    TYPECHECK_ARGS("=", args, 2, SCAM_ANY, SCAM_ANY);
    scamval* left = scamseq_get(args, 0);
    scamval* right = scamseq_get(args, 1);
    return scambool(scamval_eq(left, right));
}

scamval* builtin_gt(scamval* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    scamval* left = scamseq_get(args, 0);
    scamval* right = scamseq_get(args, 1);
    return scambool(scamval_gt(left, right));
}

scamval* builtin_lt(scamval* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    scamval* left = scamseq_get(args, 0);
    scamval* right = scamseq_get(args, 1);
    return scambool(!scamval_gt(left, right) && !scamval_eq(left, right));
}

scamval* builtin_gte(scamval* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    scamval* left = scamseq_get(args, 0);
    scamval* right = scamseq_get(args, 1);
    return scambool(scamval_gt(left, right) || scamval_eq(left, right));
}

scamval* builtin_lte(scamval* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    scamval* left = scamseq_get(args, 0);
    scamval* right = scamseq_get(args, 1);
    return scambool(!scamval_gt(left, right));
}

scamval* builtin_not(scamval* args) {
    TYPECHECK_ARGS("not", args, 1, SCAM_BOOL);
    return scambool(!scam_as_bool(scamseq_get(args, 0)));
}

void add_builtin(scamval* env, char* sym, scambuiltin_fun bltin) {
    scamdict_bind(env, scamsym(sym), scambuiltin(bltin));
}

// If a builtin doesn't change its arguments, then it should be registered
// as constant so that the evaluator doesn't bother copying the argument list
void add_const_builtin(scamval* env, char* sym, scambuiltin_fun bltin) {
    scamdict_bind(env, scamsym(sym), scambuiltin_const(bltin));
}

scamval* scamdict_builtins() {
    scamval* env = scamdict(NULL);
    add_builtin(env, "begin", builtin_begin);
    add_const_builtin(env, "-", builtin_sub);
    add_const_builtin(env, "+", builtin_add);
    add_const_builtin(env, "*", builtin_mult);
    add_const_builtin(env, "/", builtin_real_div);
    add_const_builtin(env, "//", builtin_floor_div);
    add_const_builtin(env, "%", builtin_rem);
    add_const_builtin(env, "=", builtin_eq);
    add_const_builtin(env, ">", builtin_gt);
    add_const_builtin(env, "<", builtin_lt);
    add_const_builtin(env, ">=", builtin_gte);
    add_const_builtin(env, "<=", builtin_lte);
    add_const_builtin(env, "not", builtin_not);
    // sequence functions
    add_const_builtin(env, "len", builtin_len);
    add_const_builtin(env, "empty?", builtin_empty);
    add_const_builtin(env, "head", builtin_head);
    add_builtin(env, "tail", builtin_tail);
    add_const_builtin(env, "last", builtin_last);
    add_builtin(env, "init", builtin_init);
    add_const_builtin(env, "get", builtin_get);
    add_builtin(env, "slice", builtin_slice);
    add_builtin(env, "take", builtin_take);
    add_builtin(env, "drop", builtin_drop);
    add_builtin(env, "append", builtin_append);
    add_builtin(env, "prepend", builtin_prepend);
    add_builtin(env, "concat", builtin_concat);
    add_const_builtin(env, "find", builtin_find);
    add_const_builtin(env, "rfind", builtin_rfind);
    // string functions
    add_builtin(env, "upper", builtin_upper);
    add_builtin(env, "lower", builtin_lower);
    add_builtin(env, "trim", builtin_trim);
    add_builtin(env, "split", builtin_split);
    // IO functions
    add_const_builtin(env, "print", builtin_print);
    add_const_builtin(env, "println", builtin_println);
    add_const_builtin(env, "open", builtin_open);
    add_builtin(env, "close", builtin_close);
    add_const_builtin(env, "port-good?", builtin_port_good);
    add_builtin(env, "readline", builtin_readline);
    add_builtin(env, "readchar", builtin_readchar);
    // miscellaneous functions
    add_const_builtin(env, "assert", builtin_assert);
    add_const_builtin(env, "range", builtin_range);
    add_builtin(env, "sort", builtin_sort);
    add_builtin(env, "map", builtin_map);
    add_builtin(env, "filter", builtin_filter);
    add_const_builtin(env, "id", builtin_id);
    // stdin, stdout and stderr
    scamdict_bind(env, scamsym("stdin"), scamport(stdin));
    scamdict_bind(env, scamsym("stdout"), scamport(stdout));
    scamdict_bind(env, scamsym("stderr"), scamport(stderr));
    return env;
}
