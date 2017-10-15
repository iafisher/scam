#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"

#define TYPECHECK_ARGS(name, args, n, ...) { \
    ScamVal* check_result = typecheck_args(name, args, n, ##__VA_ARGS__); \
    if (check_result) { \
        return check_result; \
    } \
}

#define TYPECHECK_ALL(name, args, min, type) { \
    ScamVal* check_result = typecheck_all(name, args, min, type); \
    if (check_result) { \
        return check_result; \
    } \
}

// Assert that no element of the args is equal to zero, unless only the first element is zero and 
// the args contains only two arguments
#define ASSERT_NO_ZEROS(args) { \
    if (ScamSeq_len(args) > 2) { \
        ScamVal* first = ScamSeq_get(args, 0); \
        if ((first->type == SCAM_INT && ScamInt_unbox((ScamInt*)first) == 0) || \
            (first->type == SCAM_DEC && ScamDec_unbox((ScamDec*)first) == 0.0)) { \
            return (ScamVal*)ScamErr_new("cannot divide by zero"); \
        } \
    } \
    for (int i = 1; i < ScamSeq_len(args); i++) { \
        ScamVal* v = ScamSeq_get(args, i); \
        if ((v->type == SCAM_INT && ScamInt_unbox((ScamInt*)v) == 0) || \
            (v->type == SCAM_DEC && ScamDec_unbox((ScamDec*)v) == 0.0)) { \
            return (ScamVal*)ScamErr_new("cannot divide by zero"); \
        } \
    } \
}

ScamVal* typecheck_args(char* name, ScamSeq* args, size_t arity, ...) {
    size_t n = ScamSeq_len(args);
    if (n != arity) {
        return (ScamVal*)ScamErr_arity(name, n, arity);
    }
    va_list vlist;
    va_start(vlist, arity);
    for (int i = 0; i < ScamSeq_len(args); i++) {
        int type_we_need = va_arg(vlist, int);
        ScamVal* v = ScamSeq_get(args, i);
        if (!ScamVal_typecheck(v, type_we_need)) {
            return (ScamVal*)ScamErr_type(name, i, v->type, type_we_need);
        }
    }
    return NULL;
}

ScamVal* typecheck_all(char* name, ScamSeq* args, int min, int type_we_need) {
    size_t n = ScamSeq_len(args);
    if (n < min) {
        return (ScamVal*)ScamErr_min_arity(name, n, min);
    }
    for (int i = 0; i < n; i++) {
        ScamVal* v = ScamSeq_get(args, i);
        if (!ScamVal_typecheck(v, type_we_need)) {
            return (ScamVal*)ScamErr_type(name, i, v->type, type_we_need);
        }
    }
    return NULL;
}

typedef long long (int_arith_func)(long long, long long);
ScamVal* generic_int_arith(char* name, ScamSeq* args, int_arith_func op) {
    TYPECHECK_ALL(name, args, 2, SCAM_INT);
    ScamInt* first = (ScamInt*)ScamSeq_get(args, 0);
    long long sum = ScamInt_unbox(first);
    for (int i = 1; i < ScamSeq_len(args); i++) {
        sum = op(sum, ScamInt_unbox((ScamInt*)ScamSeq_get(args, i)));
    }
    return (ScamVal*)ScamInt_new(sum);
}

typedef double (arith_func)(double, double);
ScamVal* generic_mixed_arith(char* name, ScamSeq* args, arith_func op, int coerce_to_double) {
    TYPECHECK_ALL(name, args, 2, SCAM_NUM);
    ScamDec* first = (ScamDec*)ScamSeq_get(args, 0);
    double sum = ScamDec_unbox(first);
    int seen_double = (first->type == SCAM_DEC) ? 1 : 0;
    for (int i = 1; i < ScamSeq_len(args); i++) {
        ScamVal* v = ScamSeq_get(args, i);
        if (v->type == SCAM_INT) {
            sum = op(sum, ScamInt_unbox((ScamInt*)v));
        } else {
            sum = op(sum, ScamDec_unbox((ScamDec*)v));
            seen_double = 1;
        }
    }
    if (seen_double || coerce_to_double) {
        return (ScamVal*)ScamDec_new(sum);
    } else {
        return (ScamVal*)ScamInt_new(sum);
    }
}

double arith_add(double x, double y) { return x + y; }
ScamVal* builtin_add(ScamSeq* args) {
    return generic_mixed_arith("+", args, arith_add, 0);
}

ScamVal* builtin_negate(ScamSeq* args) {
    TYPECHECK_ARGS("-", args, 1, SCAM_NUM);
    ScamVal* v = ScamSeq_get(args, 0);
    if (v->type == SCAM_INT) {
        return (ScamVal*)ScamInt_new(-1 * ScamInt_unbox((ScamInt*)v));
    } else {
        return (ScamVal*)ScamDec_new(-1 * ScamDec_unbox((ScamDec*)v));
    }
}

double arith_sub(double x, double y) { return x - y; }
ScamVal* builtin_sub(ScamSeq* args) {
    if (ScamSeq_len(args) == 1) {
        return builtin_negate(args);
    } else {
        return generic_mixed_arith("-", args, arith_sub, 0);
    }
}

double arith_mult(double x, double y) { return x * y; }
ScamVal* builtin_mult(ScamSeq* args) {
    return generic_mixed_arith("*", args, arith_mult, 0);
}

long long arith_rem(long long x, long long y) { return x % y; }
ScamVal* builtin_rem(ScamSeq* args) {
    ASSERT_NO_ZEROS(args);
    return generic_int_arith("%", args, arith_rem);
}

double arith_real_div(double x, double y) { return x / y; }
ScamVal* builtin_real_div(ScamSeq* args) {
    ASSERT_NO_ZEROS(args);
    return generic_mixed_arith("/", args, arith_real_div, 1);
}

long long arith_floor_div(long long x, long long y) { return x / y; }
ScamVal* builtin_floor_div(ScamSeq* args) {
    ASSERT_NO_ZEROS(args);
    return generic_int_arith("//", args, arith_floor_div);
}

ScamVal* builtin_len(ScamSeq* args) {
    TYPECHECK_ARGS("len", args, 1, SCAM_SEQ);
    ScamVal* arg = ScamSeq_get(args, 0);
    if (arg->type == SCAM_STR) {
        return (ScamVal*)ScamInt_new(ScamStr_len((ScamStr*)arg));
    } else {
        return (ScamVal*)ScamInt_new(ScamSeq_len((ScamSeq*)arg));
    } 
}

ScamVal* builtin_empty(ScamSeq* args) {
    TYPECHECK_ARGS("empty?", args, 1, SCAM_SEQ);
    ScamVal* arg = ScamSeq_get(args, 0);
    if (arg->type == SCAM_STR) {
        return (ScamVal*)ScamBool_new(ScamStr_len((ScamStr*)arg) == 0);
    } else {
        return (ScamVal*)ScamBool_new(ScamSeq_len((ScamSeq*)arg) == 0);
    } 
}

ScamVal* builtin_list_get(ScamSeq* args) {
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    size_t i = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    if (i >= 0 && i < ScamSeq_len(list_arg)) {
        return gc_copy_ScamVal(ScamSeq_get(list_arg, i));
    } else {
        return (ScamVal*)ScamErr_new("attempted sequence access out of range");
    }
}

ScamVal* builtin_str_get(ScamSeq* args) {
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    size_t i = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    if (i >= 0 && i < ScamStr_len(str_arg)) {
        return (ScamVal*)ScamStr_from_char(ScamStr_get(str_arg, i));
    } else {
        return (ScamVal*)ScamErr_new("attempted string access out of range");
    }
}

ScamVal* builtin_dict_get(ScamSeq* args) {
    ScamDict* dict_arg = (ScamDict*)ScamSeq_get(args, 0);
    ScamStr* key_arg = (ScamStr*)ScamSeq_get(args, 1);
    return ScamDict_lookup(dict_arg, key_arg);
}

ScamVal* builtin_get(ScamSeq* args) {
    TYPECHECK_ARGS("get", args, 2, SCAM_CONTAINER, SCAM_ANY);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_DICT) {
        return builtin_dict_get(args);
    } else {
        TYPECHECK_ARGS("get", args, 2, SCAM_CONTAINER, SCAM_INT);
        if (type == SCAM_STR) {
            return builtin_str_get(args);
        } else {
            return builtin_list_get(args);
        }
    }
}

ScamVal* builtin_str_slice(ScamSeq* args) {
    size_t start = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    size_t end = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 2));
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    return (ScamVal*)ScamStr_substr(str_arg, start, end);
}

ScamVal* builtin_list_slice(ScamSeq* args) {
    size_t start = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    size_t end = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 2));
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    return (ScamVal*)ScamSeq_subseq(list_arg, start, end);
}

ScamVal* builtin_slice(ScamSeq* args) {
    TYPECHECK_ARGS("slice", args, 3, SCAM_SEQ, SCAM_INT, SCAM_INT);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_slice(args);
    } else {
        return builtin_list_slice(args);
    }
}

ScamVal* builtin_str_take(ScamSeq* args) {
    size_t end = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    return (ScamVal*)ScamStr_substr(str_arg, 0, end);
}

ScamVal* builtin_list_take(ScamSeq* args) {
    size_t end = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    return (ScamVal*)ScamSeq_subseq(list_arg, 0, end);
}

ScamVal* builtin_take(ScamSeq* args) {
    TYPECHECK_ARGS("take", args, 2, SCAM_SEQ, SCAM_INT);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_take(args);
    } else {
        return builtin_list_take(args);
    }
}

ScamVal* builtin_str_drop(ScamSeq* args) {
    size_t start = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    return (ScamVal*)ScamStr_substr(str_arg, start, ScamStr_len(str_arg));
}

ScamVal* builtin_list_drop(ScamSeq* args) {
    size_t start = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    return (ScamVal*)ScamSeq_subseq(list_arg, start, ScamSeq_len(list_arg));
}

ScamVal* builtin_drop(ScamSeq* args) {
    TYPECHECK_ARGS("drop", args, 2, SCAM_SEQ, SCAM_INT);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_drop(args);
    } else {
        return builtin_list_drop(args);
    }
}

ScamVal* builtin_list_head(ScamSeq* args) {
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    if (ScamSeq_len(list_arg) > 0) {
        return gc_copy_ScamVal(ScamSeq_get(list_arg, 0));
    } else {
        return (ScamVal*)ScamErr_new("cannot take head of empty list");
    }
}

ScamVal* builtin_str_head(ScamSeq* args) {
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    if (ScamStr_len(str_arg) > 0) {
        return (ScamVal*)ScamStr_from_char(ScamStr_get(str_arg, 0));
    } else {
        return (ScamVal*)ScamErr_new("cannot take head of empty string");
    }
}

ScamVal* builtin_head(ScamSeq* args) {
    TYPECHECK_ARGS("head", args, 1, SCAM_SEQ);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_head(args);
    } else {
        return builtin_list_head(args);
    }
}

ScamVal* builtin_list_tail(ScamSeq* args) {
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    ScamSeq_delete(list_arg, 0);
    return (ScamVal*)list_arg;
}

ScamVal* builtin_str_tail(ScamSeq* args) {
    ScamStr* str_arg = (ScamStr*)ScamSeq_pop(args, 0);
    ScamStr_pop(str_arg, 0);
    return (ScamVal*)str_arg;
}

ScamVal* builtin_tail(ScamSeq* args) {
    TYPECHECK_ARGS("tail", args, 1, SCAM_SEQ);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_tail(args);
    } else {
        return builtin_list_tail(args);
    }
}

ScamVal* builtin_list_last(ScamSeq* args) {
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    if (ScamSeq_len(list_arg) > 0) {
        return gc_copy_ScamVal(ScamSeq_get(list_arg, ScamSeq_len(list_arg)-1));
    } else {
        return (ScamVal*)ScamErr_new("cannot take last of empty list");
    }
}

ScamVal* builtin_str_last(ScamSeq* args) {
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    size_t n = ScamStr_len(str_arg);
    if (n > 0) {
        return (ScamVal*)ScamStr_from_char(ScamStr_get(str_arg, n - 1));
    } else {
        return (ScamVal*)ScamErr_new("cannot take last of empty string");
    }
}

ScamVal* builtin_last(ScamSeq* args) {
    TYPECHECK_ARGS("last", args, 1, SCAM_SEQ);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_last(args);
    } else {
        return builtin_list_last(args);
    } 
}

ScamVal* builtin_list_init(ScamSeq* args) {
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    ScamSeq_delete(list_arg, ScamSeq_len(list_arg) - 1);
    return (ScamVal*)list_arg;
}

ScamVal* builtin_str_init(ScamSeq* args) {
    ScamStr* str_arg = (ScamStr*)ScamSeq_pop(args, 0);
    size_t n = ScamStr_len(str_arg);
    ScamStr_pop(str_arg, n - 1);
    return (ScamVal*)str_arg;
}

ScamVal* builtin_init(ScamSeq* args) {
    TYPECHECK_ARGS("init", args, 1, SCAM_SEQ);
    int type = ScamSeq_get(args, 0)->type;
    if (type == SCAM_STR) {
        return builtin_str_init(args);
    } else {
        return builtin_list_init(args);
    }
}

ScamVal* builtin_insert(ScamSeq* args) {
    TYPECHECK_ARGS("insert", args, 3, SCAM_LIST, SCAM_INT, SCAM_ANY);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    size_t i = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 0));
    ScamVal* to_insert = ScamSeq_pop(args, 1);
    if (i >= 0 && i <= ScamSeq_len(list_arg)) {
        ScamSeq_insert(list_arg, i, to_insert);
        return (ScamVal*)list_arg;
    } else {
        return (ScamVal*)ScamErr_new("attempted sequence access out of range");
    }
}

ScamVal* builtin_append(ScamSeq* args) {
    TYPECHECK_ARGS("append", args, 2, SCAM_LIST, SCAM_ANY);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    ScamVal* v = ScamSeq_pop(args, 0);
    ScamSeq_append(list_arg, v);
    return (ScamVal*)list_arg;
}

ScamVal* builtin_prepend(ScamSeq* args) {
    TYPECHECK_ARGS("prepend", args, 2, SCAM_ANY, SCAM_LIST);
    ScamVal* v = ScamSeq_pop(args, 0);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    ScamSeq_prepend(list_arg, v);
    return (ScamVal*)list_arg;
}

ScamVal* builtin_list_concat(ScamSeq* args) {
    ScamSeq* first_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    while (ScamSeq_len(args) > 0) {
        ScamSeq_concat(first_arg, (ScamSeq*)ScamSeq_pop(args, 0));
    }
    return (ScamVal*)first_arg;
}

ScamVal* builtin_str_concat(ScamSeq* args) {
    ScamStr* first_arg = (ScamStr*)ScamSeq_pop(args, 0);
    while (ScamSeq_len(args) > 0) {
        ScamStr_concat(first_arg, (ScamStr*)ScamSeq_pop(args, 0));
    }
    return (ScamVal*)first_arg;
}

ScamVal* builtin_concat(ScamSeq* args) {
    TYPECHECK_ALL("concat", args, 2, SCAM_SEQ);
    int narrowest_type = ScamSeq_narrowest_type(args);
    if (narrowest_type == SCAM_LIST) {
        return builtin_list_concat(args);
    } else if (narrowest_type == SCAM_STR) {
        return builtin_str_concat(args);
    } else {
        return (ScamVal*)ScamErr_new("cannot concatenate lists and strings");
    }
}

ScamVal* builtin_find(ScamSeq* args) {
    TYPECHECK_ARGS("find", args, 2, SCAM_LIST, SCAM_ANY);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    ScamVal* datum = ScamSeq_get(args, 1);
    for (int i = 0; i < ScamSeq_len(list_arg); i++) {
        if (ScamVal_eq(ScamSeq_get(list_arg, i), datum)) {
            return (ScamVal*)ScamInt_new(i);
        }
    }
    return (ScamVal*)ScamBool_new(0);
}

ScamVal* builtin_rfind(ScamSeq* args) {
    TYPECHECK_ARGS("rfind", args, 2, SCAM_LIST, SCAM_ANY);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_get(args, 0);
    ScamVal* datum = ScamSeq_get(args, 1);
    for (int i = ScamSeq_len(list_arg) - 1; i >= 0; i--) {
        if (ScamVal_eq(ScamSeq_get(list_arg, i), datum)) {
            return (ScamVal*)ScamInt_new(i);
        }
    }
    return (ScamVal*)ScamBool_new(0);
}

ScamVal* builtin_upper(ScamSeq* args) {
    TYPECHECK_ARGS("upper", args, 1, SCAM_STR);
    ScamStr* str_arg = (ScamStr*)ScamSeq_pop(args, 0);
    ScamStr_map(str_arg, toupper);
    return (ScamVal*)str_arg;
}

ScamVal* builtin_lower(ScamSeq* args) {
    TYPECHECK_ARGS("lower", args, 1, SCAM_STR);
    ScamStr* str_arg = (ScamStr*)ScamSeq_pop(args, 0);
    ScamStr_map(str_arg, tolower);
    return (ScamVal*)str_arg;
}

ScamVal* builtin_isupper(ScamSeq* args) {
    TYPECHECK_ARGS("isupper", args, 1, SCAM_STR);
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    int seen_upper = 0;
    for (size_t i = 0; i < ScamStr_len(str_arg); i++) {
        char c = ScamStr_get(str_arg, i);
        if (isalpha(c)) {
            if (!isupper(c)) {
                return (ScamVal*)ScamBool_new(0);
            } else {
                seen_upper = 1;
            }
        }
    }
    return (ScamVal*)ScamBool_new(seen_upper);
}

ScamVal* builtin_islower(ScamSeq* args) {
    TYPECHECK_ARGS("islower", args, 1, SCAM_STR);
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    int seen_lower = 0;
    for (size_t i = 0; i < ScamStr_len(str_arg); i++) {
        char c = ScamStr_get(str_arg, i);
        if (isalpha(c)) {
            if (!islower(c)) {
                return (ScamVal*)ScamBool_new(0);
            } else {
                seen_lower = 1;
            }
        }
    }
    return (ScamVal*)ScamBool_new(seen_lower);
}

ScamVal* builtin_trim(ScamSeq* args) {
    TYPECHECK_ARGS("trim", args, 1, SCAM_STR);
    ScamStr* str_arg = (ScamStr*)ScamSeq_pop(args, 0);
    size_t n = ScamStr_len(str_arg);
    // remove left whitespace
    size_t left_ws = 0;
    while (isspace(ScamStr_get(str_arg, left_ws)) && left_ws < n)
        left_ws++;
    if (left_ws > 0) {
        ScamStr_remove(str_arg, 0, left_ws);
    }
    // remove right whitespace
    n -= left_ws;
    size_t right_ws = 0;
    while (isspace(ScamStr_get(str_arg, n - (right_ws + 1))) && right_ws < n)
        right_ws++;
    ScamStr_truncate(str_arg, n - right_ws);
    return (ScamVal*)str_arg;
}

ScamVal* builtin_split(ScamSeq* args) {
    TYPECHECK_ARGS("split", args, 1, SCAM_STR);
    ScamSeq* ret = ScamList_new();
    ScamStr* str_arg = (ScamStr*)ScamSeq_get(args, 0);
    int start = 0;
    int in_word = 0;
    for (int i = 0; i < ScamStr_len(str_arg); i++) {
        if (!isspace(ScamStr_get(str_arg, i))) {
            if (!in_word) {
                in_word = 1;
                start = i;
            }
        } else {
            if (in_word) {
                in_word = 0;
                ScamSeq_append(ret, (ScamVal*)ScamStr_substr(str_arg, start, i));
            }
        }
    }
    // make sure to add the last word
    if (in_word) {
        ScamSeq_append(ret, (ScamVal*)ScamStr_substr(str_arg, start, ScamStr_len(str_arg)));
    }
    return (ScamVal*)ret;
}

ScamVal* builtin_bind(ScamSeq* args) {
    TYPECHECK_ARGS("bind", args, 3, SCAM_DICT, SCAM_ANY, SCAM_ANY);
    ScamDict* dict_arg = (ScamDict*)ScamSeq_get(args, 0);
    ScamStr* key_arg = (ScamStr*)ScamSeq_get(args, 1);
    ScamVal* val_arg = ScamSeq_get(args, 2);
    ScamDict_bind(dict_arg, key_arg, val_arg);
    return (ScamVal*)dict_arg;
}

ScamVal* builtin_list(ScamSeq* args) {
    args->type = SCAM_LIST;
    return (ScamVal*)args;
}

ScamVal* builtin_dict(ScamSeq* args) {
    TYPECHECK_ALL("dict", args, 0, SCAM_LIST);
    ScamDict* ret = ScamDict_new(NULL);
    for (size_t i = 0; i < ScamSeq_len(args); i++) {
        ScamSeq* pair = (ScamSeq*)ScamSeq_get(args, i);
        if (ScamSeq_len(pair) == 2) {
            ScamStr* key = (ScamStr*)ScamSeq_get(pair, 0);
            ScamVal* val = ScamSeq_get(pair, 1);
            ScamDict_bind(ret, key, val);
        } else {
            gc_unset_root((ScamVal*)ret);
            return (ScamVal*)ScamErr_new("'dict' expects each argument to be a pair");
        }
    }
    return (ScamVal*)ret;
}

ScamVal* builtin_str(ScamSeq* args) {
    TYPECHECK_ARGS("str", args, 1, SCAM_ANY);
    return (ScamVal*)ScamStr_no_copy(ScamVal_to_str((ScamSeq_get(args, 0))));
}

ScamVal* builtin_repr(ScamSeq* args) {
    TYPECHECK_ARGS("repr", args, 1, SCAM_ANY);
    return (ScamVal*)ScamStr_no_copy(ScamVal_to_repr(ScamSeq_get(args, 0)));
}

ScamVal* builtin_print(ScamSeq* args) {
    TYPECHECK_ARGS("print", args, 1, SCAM_ANY);
    ScamVal* arg = ScamSeq_get(args, 0);
    if (arg->type != SCAM_STR) {
        ScamVal_print(arg);
    } else {
        printf("%s", ScamStr_unbox((ScamStr*)arg));
    }
    return ScamNull_new();
}

ScamVal* builtin_println(ScamSeq* args) {
    TYPECHECK_ARGS("println", args, 1, SCAM_ANY);
    ScamVal* arg = ScamSeq_get(args, 0);
    if (arg->type != SCAM_STR) {
        ScamVal_println(arg);
    } else {
        printf("%s\n", ScamStr_unbox((ScamStr*)arg));
    }
    return ScamNull_new();
}

ScamVal* builtin_open(ScamSeq* args) {
    TYPECHECK_ARGS("open", args, 2, SCAM_STR, SCAM_STR);
    const char* fname = ScamStr_unbox((ScamStr*)ScamSeq_get(args, 0));
    const char* mode = ScamStr_unbox((ScamStr*)ScamSeq_get(args, 1));
    FILE* fp = fopen(fname, mode);
    return (ScamVal*)ScamPort_new(fp);
}

ScamVal* builtin_close(ScamSeq* args) {
    TYPECHECK_ARGS("close", args, 1, SCAM_PORT);
    ScamPort* port_arg = (ScamPort*)ScamSeq_get(args, 0);
    if (ScamPort_status(port_arg) == SCAMPORT_OPEN) {
        fclose(ScamPort_unbox(port_arg));
        ScamPort_set_status(port_arg, SCAMPORT_CLOSED);
        return ScamNull_new();
    } else {
        return (ScamVal*)ScamErr_new("port is already closed");
    }
}

ScamVal* builtin_port_good(ScamSeq* args) {
    TYPECHECK_ARGS("port-good?", args, 1, SCAM_PORT);
    ScamPort* port_arg = (ScamPort*)ScamSeq_get(args, 0);
    if (ScamPort_status(port_arg) == SCAMPORT_OPEN) {
        FILE* fp = ScamPort_unbox(port_arg);
        return (ScamVal*)ScamBool_new(!ferror(fp) && !feof(fp));
    } else {
        return (ScamVal*)ScamBool_new(0);
    }
}

ScamVal* builtin_readline(ScamSeq* args) {
    if (ScamSeq_len(args) == 0) {
        return (ScamVal*)ScamStr_read(stdin);
    } else {
        TYPECHECK_ARGS("readline", args, 1, SCAM_PORT);
        ScamPort* port_arg = (ScamPort*)ScamSeq_get(args, 0);
        if (ScamPort_status(port_arg) == SCAMPORT_OPEN) {
            return (ScamVal*)ScamStr_read(ScamPort_unbox(port_arg));
        } else {
            return (ScamVal*)ScamErr_new("cannot read from closed port");
        }
    }
}

ScamVal* builtin_readchar(ScamSeq* args) {
    TYPECHECK_ARGS("readchar", args, 1, SCAM_PORT);
    ScamPort* port_arg = (ScamPort*)ScamSeq_get(args, 0);
    if (ScamPort_status(port_arg) == SCAMPORT_OPEN) {
        char c = fgetc(ScamPort_unbox(port_arg));
        if (c != EOF) {
            return (ScamVal*)ScamStr_from_char(c);
        } else {
            return (ScamVal*)ScamErr_eof();
        }
    } else {
        return (ScamVal*)ScamErr_new("cannot read from closed port");
    }
}

ScamVal* builtin_ceil(ScamSeq* args) {
    TYPECHECK_ARGS("ceil", args, 1, SCAM_DEC);
    double d = ScamDec_unbox((ScamDec*)ScamSeq_get(args, 0));
    return (ScamVal*)ScamInt_new(ceil(d));
}

ScamVal* builtin_floor(ScamSeq* args) {
    TYPECHECK_ARGS("floor", args, 1, SCAM_DEC);
    double d = ScamDec_unbox((ScamDec*)ScamSeq_get(args, 0));
    return (ScamVal*)ScamInt_new(floor(d));
}

ScamVal* builtin_divmod(ScamSeq* args) {
    TYPECHECK_ARGS("divmod", args, 2, SCAM_INT, SCAM_INT);
    long long dividend = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 0));
    long long divisor = ScamInt_unbox((ScamInt*)ScamSeq_get(args, 1));
    lldiv_t res = lldiv(dividend, divisor);
    ScamSeq* ret = ScamList_new();
    ScamSeq_append(ret, (ScamVal*)ScamInt_new(res.quot));
    ScamSeq_append(ret, (ScamVal*)ScamInt_new(res.rem));
    return (ScamVal*)ret;
}

ScamVal* builtin_abs(ScamSeq* args) {
    TYPECHECK_ARGS("abs", args, 1, SCAM_NUM);
    ScamVal* num_arg = ScamSeq_get(args, 0);
    if (num_arg->type == SCAM_DEC) {
        double d = ScamDec_unbox((ScamDec*)num_arg);
        return (ScamVal*)ScamDec_new(fabs(d));
    } else {
        long long n = ScamInt_unbox((ScamInt*)num_arg);
        return (ScamVal*)ScamInt_new(llabs(n));
    }
}

ScamVal* builtin_sqrt(ScamSeq* args) {
    TYPECHECK_ARGS("sqrt", args, 1, SCAM_NUM);
    ScamDec* num_arg = (ScamDec*)ScamSeq_get(args, 0);
    double d = ScamDec_unbox(num_arg);
    if (d >= 0) {
        return (ScamVal*)ScamDec_new(sqrt(d));
    } else {
        return (ScamVal*)ScamErr_new("cannot take square root of a negative number");
    }
}

ScamVal* builtin_pow(ScamSeq* args) {
    TYPECHECK_ARGS("pow", args, 2, SCAM_NUM, SCAM_NUM);
    ScamVal* base_arg = ScamSeq_get(args, 0);
    ScamVal* exp_arg = ScamSeq_get(args, 1);
    double base = ScamDec_unbox((ScamDec*)base_arg);
    double exp = ScamDec_unbox((ScamDec*)exp_arg);
    if (base_arg->type == SCAM_INT && exp_arg->type == SCAM_INT) {
        return (ScamVal*)ScamInt_new(pow(base, exp));
    } else {
        return (ScamVal*)ScamDec_new(pow(base, exp));
    }
}

ScamVal* builtin_ln(ScamSeq* args) {
    TYPECHECK_ARGS("ln", args, 1, SCAM_NUM);
    ScamDec* num_arg = (ScamDec*)ScamSeq_get(args, 0);
    double d = ScamDec_unbox(num_arg);
    return (ScamVal*)ScamDec_new(log(d));
}

ScamVal* builtin_log(ScamSeq* args) {
    TYPECHECK_ARGS("log", args, 2, SCAM_NUM, SCAM_NUM);
    ScamDec* num_arg = (ScamDec*)ScamSeq_get(args, 0);
    ScamDec* base_arg = (ScamDec*)ScamSeq_get(args, 1);
    double d = ScamDec_unbox(num_arg);
    double base = ScamDec_unbox(base_arg);
    return (ScamVal*)ScamDec_new(log(d) / log(base));
}

ScamVal* builtin_assert(ScamSeq* args) {
    TYPECHECK_ARGS("assert", args, 1, SCAM_BOOL);
    ScamInt* cond = (ScamInt*)ScamSeq_get(args, 0);
    if (ScamBool_unbox(cond)) {
        return (ScamVal*)ScamBool_new(1);
    } else {
        return (ScamVal*)ScamErr_new("failed assert");
    }
}

ScamVal* builtin_range(ScamSeq* args) {
    TYPECHECK_ARGS("range", args, 2, SCAM_INT, SCAM_INT);
    ScamInt* lower = (ScamInt*)ScamSeq_get(args, 0);
    ScamInt* upper = (ScamInt*)ScamSeq_get(args, 1);
    if (ScamInt_unbox(lower) <= ScamInt_unbox(upper)) {
        int count = ScamInt_unbox(lower);
        ScamSeq* ret = ScamList_new();
        while (count < ScamInt_unbox(upper)) {
            ScamSeq_append(ret, (ScamVal*)ScamInt_new(count));
            count++;
        }
        return (ScamVal*)ret;
    } else {
        return (ScamVal*)ScamErr_new("lower bound must be less than or equal to upper bound in function 'range'");
    }
}

int ScamVal_cmp(const void* a, const void* b) {
    const ScamVal* v1 = *(const ScamVal**)a;
    const ScamVal* v2 = *(const ScamVal**)b;
    if (ScamVal_gt(v1, v2) == 1) {
        return 1;
    } else if (ScamVal_eq(v1, v2) == 1) {
        return 0;
    } else {
        return -1;
    }
}

ScamVal* builtin_sort(ScamSeq* args) {
    TYPECHECK_ARGS("sort", args, 1, SCAM_LIST);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    qsort(list_arg->arr, ScamSeq_len(list_arg), sizeof *list_arg->arr, ScamVal_cmp);
    return (ScamVal*)list_arg;
}

ScamVal* builtin_map(ScamSeq* args) {
    TYPECHECK_ARGS("map", args, 2, SCAM_FUNCTION, SCAM_LIST);
    ScamVal* fun = ScamSeq_pop(args, 0);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    for (size_t i = 0; i < ScamSeq_len(list_arg); i++) {
        ScamVal* v = ScamSeq_get(list_arg, i);
        ScamSeq* arglist = ScamExpr_from(1, v);
        ScamVal* res = eval_apply(fun, arglist);
        gc_unset_root((ScamVal*)arglist);
        if (res->type != SCAM_ERR) {
            ScamSeq_set(list_arg, i, res);
        } else {
            gc_unset_root(fun);
            gc_unset_root((ScamVal*)list_arg);
            return res;
        }
    }
    gc_unset_root(fun);
    return (ScamVal*)list_arg;
}

ScamVal* builtin_filter(ScamSeq* args) {
    TYPECHECK_ARGS("filter", args, 2, SCAM_FUNCTION, SCAM_LIST);
    ScamVal* fun = ScamSeq_pop(args, 0);
    ScamSeq* list_arg = (ScamSeq*)ScamSeq_pop(args, 0);
    for (size_t i = 0; i < ScamSeq_len(list_arg); i++) {
        ScamVal* v = ScamSeq_get(list_arg, i);
        ScamSeq* arglist = ScamExpr_from(1, v);
        ScamVal* cond = eval_apply(fun, arglist);
        gc_unset_root((ScamVal*)arglist);
        if (cond->type == SCAM_BOOL) {
            if (!ScamBool_unbox((ScamInt*)cond)) {
                ScamSeq_delete(list_arg, i);
            }
            gc_unset_root(cond);
        } else if (cond->type == SCAM_ERR) {
            return cond;
        } else {
            gc_unset_root(cond);
            return (ScamVal*)ScamErr_new("'filter' predicate should return boolean");
        }
    }
    gc_unset_root(fun);
    return (ScamVal*)list_arg;
}

ScamVal* builtin_id(ScamSeq* args) {
    TYPECHECK_ARGS("id", args, 1, SCAM_ANY);
    return (ScamVal*)ScamInt_new((long long)ScamSeq_get(args, 0));
}

ScamVal* builtin_begin(ScamSeq* args) {
    TYPECHECK_ALL("begin", args, 1, SCAM_ANY);
    return ScamSeq_pop(args, ScamSeq_len(args) - 1);
}

ScamVal* builtin_eq(ScamSeq* args) {
    TYPECHECK_ARGS("=", args, 2, SCAM_ANY, SCAM_ANY);
    ScamVal* left = ScamSeq_get(args, 0);
    ScamVal* right = ScamSeq_get(args, 1);
    return (ScamVal*)ScamBool_new(ScamVal_eq(left, right));
}

ScamVal* builtin_gt(ScamSeq* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    ScamVal* left = ScamSeq_get(args, 0);
    ScamVal* right = ScamSeq_get(args, 1);
    return (ScamVal*)ScamBool_new(ScamVal_gt(left, right));
}

ScamVal* builtin_lt(ScamSeq* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    ScamVal* left = ScamSeq_get(args, 0);
    ScamVal* right = ScamSeq_get(args, 1);
    return (ScamVal*)ScamBool_new(!ScamVal_gt(left, right) && !ScamVal_eq(left, right));
}

ScamVal* builtin_gte(ScamSeq* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    ScamVal* left = ScamSeq_get(args, 0);
    ScamVal* right = ScamSeq_get(args, 1);
    return (ScamVal*)ScamBool_new(ScamVal_gt(left, right) || ScamVal_eq(left, right));
}

ScamVal* builtin_lte(ScamSeq* args) {
    TYPECHECK_ARGS(">", args, 2, SCAM_CMP, SCAM_CMP);
    ScamVal* left = ScamSeq_get(args, 0);
    ScamVal* right = ScamSeq_get(args, 1);
    return (ScamVal*)ScamBool_new(!ScamVal_gt(left, right));
}

ScamVal* builtin_not(ScamSeq* args) {
    TYPECHECK_ARGS("not", args, 1, SCAM_BOOL);
    return (ScamVal*)ScamBool_new(!ScamBool_unbox((ScamInt*)ScamSeq_get(args, 0)));
}

void add_builtin(ScamDict* env, char* sym, scambuiltin_fun bltin) {
    ScamDict_bind(env, ScamSym_new(sym), (ScamVal*)ScamBuiltin_new(bltin));
}

// If a builtin doesn't change its arguments, then it should be registered as constant so that the 
// evaluator doesn't bother copying the argument list
void add_const_builtin(ScamDict* env, char* sym, scambuiltin_fun bltin) {
    ScamDict_bind(env, ScamSym_new(sym), (ScamVal*)ScamBuiltin_new_const(bltin));
}

ScamDict* ScamDict_builtins(void) {
    ScamDict* env = ScamDict_new(NULL);
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
    add_builtin(env, "insert", builtin_insert);
    add_builtin(env, "append", builtin_append);
    add_builtin(env, "prepend", builtin_prepend);
    add_builtin(env, "concat", builtin_concat);
    add_const_builtin(env, "find", builtin_find);
    add_const_builtin(env, "rfind", builtin_rfind);
    // string functions
    add_builtin(env, "upper", builtin_upper);
    add_builtin(env, "lower", builtin_lower);
    add_const_builtin(env, "isupper", builtin_isupper);
    add_const_builtin(env, "islower", builtin_islower);
    add_builtin(env, "trim", builtin_trim);
    add_builtin(env, "split", builtin_split);
    // dictionary functions
    add_builtin(env, "bind", builtin_bind);
    // constructors
    add_const_builtin(env, "list", builtin_list);
    add_const_builtin(env, "dict", builtin_dict);
    add_const_builtin(env, "str", builtin_str);
    add_const_builtin(env, "repr", builtin_repr);
    // IO functions
    add_const_builtin(env, "print", builtin_print);
    add_const_builtin(env, "println", builtin_println);
    add_const_builtin(env, "open", builtin_open);
    add_builtin(env, "close", builtin_close);
    add_const_builtin(env, "port-good?", builtin_port_good);
    add_builtin(env, "readline", builtin_readline);
    add_builtin(env, "readchar", builtin_readchar);
    // math functions
    add_const_builtin(env, "ceil", builtin_ceil);
    add_const_builtin(env, "floor", builtin_floor);
    add_const_builtin(env, "divmod", builtin_divmod);
    add_const_builtin(env, "abs", builtin_abs);
    add_const_builtin(env, "sqrt", builtin_sqrt);
    add_const_builtin(env, "pow", builtin_pow);
    add_const_builtin(env, "ln", builtin_ln);
    add_const_builtin(env, "log", builtin_log);
    // miscellaneous functions
    add_const_builtin(env, "assert", builtin_assert);
    add_const_builtin(env, "range", builtin_range);
    add_builtin(env, "sort", builtin_sort);
    add_builtin(env, "map", builtin_map);
    add_builtin(env, "filter", builtin_filter);
    add_const_builtin(env, "id", builtin_id);
    // stdin, stdout and stderr
    ScamDict_bind(env, ScamSym_new("stdin"), (ScamVal*)ScamPort_new(stdin));
    ScamDict_bind(env, ScamSym_new("stdout"), (ScamVal*)ScamPort_new(stdout));
    ScamDict_bind(env, ScamSym_new("stderr"), (ScamVal*)ScamPort_new(stderr));
    return env;
}
