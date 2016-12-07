#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scamval.h"

#define MAX_ERROR_SIZE 100

array* array_init() {
    array* ret = malloc(sizeof(array));
    if (ret != NULL) {
        ret->count = 0;
        ret->root = NULL;
    }
    return ret;
}

void array_append(array* arr, scamval* v) {
    size_t new_sz = arr->count + 1;
    arr->root = realloc(arr->root, sizeof(scamval*) * new_sz);
    if (arr->root) {
        arr->count = new_sz;
        arr->root[new_sz - 1] = v;
    } else {
        arr->count = 0;
    }
}

void array_prepend(array* arr, scamval* v) {
    size_t new_sz = arr->count + 1;
    arr->root = realloc(arr->root, sizeof(scamval*) * new_sz);
    if (arr->root) {
        arr->count = new_sz;
        for (size_t i = new_sz - 1; i > 0; i--) {
            arr->root[i] = arr->root[i - 1];
        }
        arr->root[0] = v;
    } else {
        arr->count = 0;
    }
}

array* array_copy(array* arr) {
    array* ret = array_init();
    if (arr && ret) {
        ret->root = malloc(sizeof(scamval*) * arr->count);
        if (arr->root) {
            ret->count = arr->count;
            for (int i = 0; i < arr->count; i++) {
                ret->root[i] = scamval_copy(arr->root[i]);
            }
        }
        return ret;
    } else {
        // either passed a NULL pointer, or malloc failed
        return NULL;
    }
}

void array_set(array* arr, size_t i, scamval* v) {
    if (i >= 0 && i < arr->count) {
        scamval_free(arr->root[i]);
        arr->root[i] = v;
    }
}

scamval* array_get(array* arr, size_t i) {
    if (i >= 0 && i < arr->count) {
        return arr->root[i];
    } else {
        return scamval_err("attempted array access out of range");
    }
}

scamval* array_pop(array* arr, size_t i) {
    if (i >= 0 && i < arr->count) {
        scamval* ret = arr->root[i];
        for (size_t j = i; j < arr->count - 1; j++) {
            arr->root[j] = arr->root[j + 1];
        }
        arr->count--;
        return ret;
    } else {
        return scamval_err("attempted array access out of range");
    }
}

size_t array_len(array* arr) {
    return arr->count;
}

void array_free(array* arr) {
    if (arr) {
        if (arr->root) {
            for (int i = 0; i < arr->count; i++) {
                scamval_free(arr->root[i]);
            }
            free(arr->root);
        }
        free(arr);
    }
}

void scamval_append(scamval* seq, scamval* v) {
    array_append(seq->vals.arr, v);
}

void scamval_prepend(scamval* seq, scamval* v) {
    array_prepend(seq->vals.arr, v);
}

size_t scamval_len(scamval* seq) {
    return seq->vals.arr->count;
}

void scamval_set(scamval* seq, size_t i, scamval* v) {
    array_set(seq->vals.arr, i, v);
}

scamval* scamval_get(scamval* seq, size_t i) {
    return array_get(seq->vals.arr, i);
}

scamval* scamval_pop(scamval* seq, size_t i) {
    return array_pop(seq->vals.arr, i);
}

scamval* scamval_int(long long n) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_INT;
        ret->vals.n = n;
    }
    return ret;
}

scamval* scamval_dec(double d) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_DEC;
        ret->vals.d = d;
    }
    return ret;
}

scamval* scamval_bool(int b) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_BOOL;
        ret->vals.n = b;
    }
    return ret;
}

// Make a scamval that is internally an array (lists, quotes and code)
scamval* scamval_internal_array(int type) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = type;
        ret->vals.arr = array_init();
    }
    return ret;
}

scamval* scamval_list() {
    return scamval_internal_array(SCAM_LIST);
}

scamval* scamval_code() {
    return scamval_internal_array(SCAM_CODE);
}

scamval* scamval_quote() {
    return scamval_internal_array(SCAM_QUOTE);
}

// Make a scamval that is internally a string (strings, symbols and errors)
scamval* scamval_internal_str(int type, char* s) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = type;
        ret->vals.s = malloc(strlen(s) + 1);
        if (ret->vals.s)
            strcpy(ret->vals.s, s);
    }
    return ret;
}

scamval* scamval_str(char* s) {
    return scamval_internal_str(SCAM_STR, s);
}

scamval* scamval_sym(char* s) {
    return scamval_internal_str(SCAM_SYM, s);
}

scamval* scamval_err(char* format, ...) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_ERR;
        va_list vlist;
        va_start(vlist, format);
        ret->vals.s = malloc(MAX_ERROR_SIZE);
        if (ret->vals.s) {
            vsnprintf(ret->vals.s, MAX_ERROR_SIZE, format, vlist);
            va_end(vlist);
        }
    }
    return ret;
}

scamval* scamval_function(scamenv* env, scamval* parameters, scamval* body) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_FUNCTION;
        // probably going to need to change this at some point
        ret->vals.fun = malloc(sizeof(scamfun));
        if (ret->vals.fun) {
            ret->vals.fun->env = scamenv_init(env);
            ret->vals.fun->parameters = parameters;
            ret->vals.fun->body = body;
        }
    }
    return ret;
}

scamval* scamval_builtin(scambuiltin* bltin) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_BUILTIN;
        ret->vals.bltin = bltin;
    }
    return ret;
}

scamval* scamval_port(FILE* fp) {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_PORT;
        ret->vals.port = fp;
    }
    return ret;
}

scamval* scamval_null() {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_NULL;
    }
    return ret;
}

scamval* scamval_copy(scamval* v) {
    scamval* ret = malloc(sizeof(scamval));
    if (v && ret) {
        ret->type = v->type;
        switch (v->type) {
            case SCAM_BOOL:
            case SCAM_INT: 
                ret->vals.n = v->vals.n; break;
            case SCAM_DEC: 
                ret->vals.d = v->vals.d; break;
            case SCAM_LIST:
            case SCAM_CODE:
            case SCAM_QUOTE:
                ret->vals.arr = array_copy(v->vals.arr); break;
            case SCAM_STR:
            case SCAM_SYM:
            case SCAM_ERR:
                ret->vals.s = malloc(strlen(v->vals.s) + 1);
                if (ret->vals.s)
                    strcpy(ret->vals.s, v->vals.s);
                break;
            case SCAM_FUNCTION:
                ret->vals.fun = malloc(sizeof(scamfun));
                if (ret->vals.fun) {
                    ret->vals.fun->env = scamenv_copy(v->vals.fun->env);
                    ret->vals.fun->parameters = scamval_copy(v->vals.fun->parameters);
                    ret->vals.fun->body = scamval_copy(v->vals.fun->body);
                }
                break;
            case SCAM_BUILTIN:
                ret->vals.bltin = v->vals.bltin;
            case SCAM_PORT:
                ret->vals.port = v->vals.port;
        }
        return ret;
    } else {
        // either passed a NULL pointer, or malloc failed
        return NULL;
    }
}

int is_numeric_type(scamval* v) {
    return v->type == SCAM_INT || v->type == SCAM_DEC;
}

int scamval_numeric_eq(scamval* v1, scamval* v2) {
    if (v1->type == SCAM_INT) {
        if (v2->type == SCAM_INT) {
            return v1->vals.n == v2->vals.n;
        } else {
            return v1->vals.n == v2->vals.d;
        }
    } else {
        if (v2->type == SCAM_INT) {
            return v1->vals.d == v2->vals.n;
        } else {
            return v1->vals.d == v2->vals.d;
        }
    }
}

int scamval_list_eq(scamval* v1, scamval* v2) {
    size_t n1 = scamval_len(v1);
    size_t n2 = scamval_len(v2);
    if (n1 == n2) {
        for (int i = 0; i < n1; i++) {
            if (!scamval_eq(scamval_get(v1, i), scamval_get(v2, i))) {
                return 0;
            }
        }
        return 1;
    } else {
        return 0;
    }
}

int scamval_eq(scamval* v1, scamval* v2) {
    if (is_numeric_type(v1) && is_numeric_type(v2)) {
        return scamval_numeric_eq(v1, v2);
    } else if (v1->type == v2->type) {
        switch (v1->type) {
            case SCAM_BOOL:
                return v1->vals.n == v2->vals.n;
            case SCAM_CODE:
            case SCAM_QUOTE:
            case SCAM_LIST:
                return scamval_list_eq(v1, v2);
            case SCAM_SYM:
            case SCAM_STR:
                return (strcmp(v1->vals.s, v2->vals.s) == 0);
            default:
                return 0;
        }
    } else {
        return 0;
    }
}

void scamval_free(scamval* v) {
    if (!v) return;
    switch (v->type) {
        case SCAM_LIST:
        case SCAM_QUOTE:
        case SCAM_CODE:
            array_free(v->vals.arr); break;
        case SCAM_STR:
        case SCAM_SYM:
        case SCAM_ERR:
            if (v->vals.s) free(v->vals.s); break;
        case SCAM_PORT:
            if (v->vals.port != SCAM_CLOSED_FILE) {
                fclose(v->vals.port);
            }
            break;
        case SCAM_FUNCTION:
            scamenv_free(v->vals.fun->env);
            scamval_free(v->vals.fun->parameters);
            scamval_free(v->vals.fun->body);
            free(v->vals.fun);
            break;
    }
    free(v);
}

scamenv* scamenv_init(scamenv* enclosing) {
    scamenv* ret = malloc(sizeof(scamenv));
    if (ret) {
        ret->enclosing = enclosing;
        ret->syms = array_init();
        ret->vals = array_init();
    }
    return ret;
}

void scamenv_bind(scamenv* env, scamval* sym, scamval* val) {
    array_append(env->syms, scamval_copy(sym));
    array_append(env->vals, scamval_copy(val));
}

scamenv* scamenv_copy(scamenv* env) {
    scamenv* ret = malloc(sizeof(scamenv));
    if (ret) {
        ret->enclosing = env->enclosing;
        ret->syms = array_copy(env->syms);
        ret->vals = array_copy(env->vals);
    }
    return ret;
}

void scamenv_free(scamenv* env) {
    if (env) {
        // do I need to free the enclosing env too?
        array_free(env->syms);
        array_free(env->vals);
        free(env);
    }
}

scamval* scamenv_lookup(scamenv* env, scamval* name) {
    for (int i = 0; i < env->syms->count; i++) {
        if (strcmp(env->syms->root[i]->vals.s, name->vals.s) == 0) {
            return scamval_copy(env->vals->root[i]);
        }
    }
    if (env->enclosing != NULL) {
        return scamenv_lookup(env->enclosing, name);
    } else {
        return scamval_err("unbound variable '%s'", name->vals.s);
    }
}

void array_print(array* arr, char* open, char* close) {
    if (!arr) return;
    printf("%s", open);
    for (int i = 0; i < arr->count; i++) {
        scamval_print(arr->root[i]);
        if (i != arr->count - 1)
            printf(" ");
    }
    printf("%s", close);
}

void scamval_print(scamval* v) {
    if (!v) return;
    switch (v->type) {
        case SCAM_INT: printf("%lli", v->vals.n); break;
        case SCAM_DEC: printf("%f", v->vals.d); break;
        case SCAM_BOOL: printf("%s", v->vals.n ? "true" : "false"); break;
        case SCAM_LIST: array_print(v->vals.arr, "[", "]"); break;
        case SCAM_CODE: array_print(v->vals.arr, "(", ")"); break;
        case SCAM_QUOTE: array_print(v->vals.arr, "{", "}"); break;
        case SCAM_FUNCTION:
        case SCAM_BUILTIN: printf("<Scam function>"); break;
        case SCAM_PORT: printf("<Scam port>"); break;
        case SCAM_STR: printf("\"%s\"", v->vals.s); break;
        case SCAM_SYM: printf("%s", v->vals.s); break;
        case SCAM_ERR: printf("Error: %s", v->vals.s); break;
    }
}

void scamval_println(scamval* v) {
    if (!v || v->type == SCAM_NULL) return;
    scamval_print(v);
    printf("\n");
}

const char* scamval_type_name(int type) {
    switch (type) {
        case SCAM_INT: return "SCAM_INT";
        case SCAM_DEC: return "SCAM_DEC";
        case SCAM_BOOL: return "SCAM_BOOL";
        case SCAM_LIST: return "SCAM_LIST";
        case SCAM_STR: return "SCAM_STR";
        case SCAM_QUOTE: return "SCAM_QUOTE";
        case SCAM_FUNCTION: return "SCAM_FUNCTION";
        case SCAM_PORT: return "SCAM_PORT";
        case SCAM_BUILTIN: return "SCAM_BUILTIN";
        case SCAM_CODE: return "SCAM_CODE";
        case SCAM_SYM: return "SCAM_SYM";
        case SCAM_ERR: return "SCAM_ERR";
        case SCAM_NULL: return "SCAM_NULL";
        default: return "bad scamval type";
    }
}
