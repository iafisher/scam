#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scamval.h"

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

scamval* scamval_err(char* s) {
    return scamval_internal_str(SCAM_ERR, s);
}

scamval* scamval_function() {
    scamval* ret = malloc(sizeof(scamval));
    if (ret) {
        ret->type = SCAM_FUNCTION;
        // probably going to need to change this at some point
        ret->vals.fun = NULL;
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
                printf("Warning: trying to copy a function\n");
                ret->vals.fun = v->vals.fun;
            case SCAM_BUILTIN:
                printf("Warning: trying to copy a builtin\n");
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
            if (v->vals.port != SCAM_CLOSED_FILE) fclose(v->vals.port);
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

void scamenv_bind(scamenv* env, char* sym, scamval* val) {
    array_append(env->syms, scamval_sym(sym));
    array_append(env->vals, scamval_copy(val));
}

void scamenv_free(scamenv* env) {
    if (env) {
        // do I need to free the enclosing env too?
        array_free(env->syms);
        array_free(env->vals);
        free(env);
    }
}

scamval* scamenv_lookup(scamenv* env, char* name) {
    for (int i = 0; i < env->syms->count; i++) {
        if (strcmp(env->syms->root[i]->vals.s, name) == 0) {
            return env->vals->root[i];
        }
    }
    return scamval_err("failed lookup");
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
    scamval_print(v);
    printf("\n");
}
