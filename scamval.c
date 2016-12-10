#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scamval.h"

#define MAX_ERROR_SIZE 100
#define ARRAY_SIZE_INITIAL 5
#define ARRAY_SIZE_INCREMENT 5

// a wrapper around malloc that exits the program if malloc returns NULL
void* my_malloc(size_t size) {
    void* ret = malloc(size);
    if (ret == NULL) {
        fputs("malloc returned a NULL pointer... exiting program\n", stderr);
        exit(EXIT_FAILURE);
    }
    return ret;
}

// a wrapper around realloc that exits the program if realloc returns NULL
void* my_realloc(void* ptr, size_t size) {
    void* ret = realloc(ptr, size);
    if (ret == NULL) {
        fputs("realloc returned a NULL pointer... exiting program", stderr);
        exit(EXIT_FAILURE);
    }
    return ret;
}

array* array_init() {
    array* ret = my_malloc(sizeof *ret);
    ret->count = 0;
    ret->mem_size = 0;
    ret->root = NULL;
    return ret;
}

void array_grow(array* arr) {
    if (arr->root == NULL) {
        arr->mem_size = ARRAY_SIZE_INITIAL;
        arr->root = my_malloc(arr->mem_size * sizeof *arr->root);
    } else {
        arr->mem_size += ARRAY_SIZE_INCREMENT;
        arr->root = my_realloc(arr->root, arr->mem_size * sizeof *arr->root);
    }
}

void array_append(array* arr, scamval* v) {
    size_t new_sz = arr->count + 1;
    if (new_sz > arr->mem_size) {
        array_grow(arr);
    }
    arr->count = new_sz;
    arr->root[new_sz - 1] = v;
}

void array_prepend(array* arr, scamval* v) {
    size_t new_sz = arr->count + 1;
    if (new_sz > arr->mem_size) {
        array_grow(arr);
    }
    arr->count = new_sz;
    for (size_t i = new_sz - 1; i > 0; i--) {
        arr->root[i] = arr->root[i - 1];
    }
    arr->root[0] = v;
}

array* array_copy(const array* arr) {
    array* ret = array_init();
    ret->root = my_malloc(arr->count * sizeof *ret->root);
    ret->count = arr->count;
    ret->mem_size = arr->count;
    for (int i = 0; i < arr->count; i++) {
        ret->root[i] = scamval_copy(arr->root[i]);
    }
    return ret;
}

void array_set(array* arr, size_t i, scamval* v) {
    if (i >= 0 && i < arr->count) {
        arr->root[i] = v;
    }
}

scamval* array_get(const array* arr, size_t i) {
    if (i >= 0 && i < arr->count) {
        return arr->root[i];
    } else {
        return scamerr("attempted array access out of range");
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
        return scamerr("attempted array access out of range");
    }
}

size_t array_len(const array* arr) {
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

size_t scamval_len(const scamval* seq) {
    return seq->vals.arr->count;
}

void scamval_set(scamval* seq, size_t i, scamval* v) {
    array_set(seq->vals.arr, i, v);
}

void scamval_replace(scamval* seq, size_t i, scamval* v) {
    scamval_free(scamval_get(seq, i));
    scamval_set(seq, i, v);
}

scamval* scamval_get(const scamval* seq, size_t i) {
    return array_get(seq->vals.arr, i);
}

scamval* scamval_pop(scamval* seq, size_t i) {
    return array_pop(seq->vals.arr, i);
}

scamval* scamint(long long n) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_INT;
    ret->vals.n = n;
    return ret;
}

scamval* scamdec(double d) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_DEC;
    ret->vals.d = d;
    return ret;
}

scamval* scambool(int b) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_BOOL;
    ret->vals.n = b;
    return ret;
}

// Make a scamval that is internally an array (lists, quotes and code)
scamval* scam_internal_array(int type) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = type;
    ret->vals.arr = array_init();
    return ret;
}

scamval* scamlist() {
    return scam_internal_array(SCAM_LIST);
}

scamval* scamcode() {
    return scam_internal_array(SCAM_CODE);
}

scamval* scamquote() {
    return scam_internal_array(SCAM_QUOTE);
}

// Make a scamval that is internally a string (strings, symbols and errors)
scamval* scam_internal_str(int type, const char* s) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = type;
    ret->vals.s = my_malloc(strlen(s) + 1);
    strcpy(ret->vals.s, s);
    return ret;
}

scamval* scamstr(const char* s) {
    return scam_internal_str(SCAM_STR, s);
}

scamval* scamsym(const char* s) {
    return scam_internal_str(SCAM_SYM, s);
}

scamval* scamerr(const char* format, ...) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_ERR;
    va_list vlist;
    va_start(vlist, format);
    ret->vals.s = my_malloc(MAX_ERROR_SIZE);
    vsnprintf(ret->vals.s, MAX_ERROR_SIZE, format, vlist);
    va_end(vlist);
    return ret;
}

scamval* scamerr_arity(const char* name, size_t got, size_t expected) {
    return scamerr("'%s' got %d arg(s), expected %d", name, got, expected);
}

scamval* scamerr_min_arity(const char* name, size_t got, size_t expected) {
    return scamerr("'%s' got %d arg(s), expected at least %d", name, got, 
                   expected);
}

scamval* scamerr_type(const char* name, size_t pos, int given_type, 
                      int req_type) {
    return scamerr("'%s' got %s as arg %d, expected %s", name, 
                   scamtype_name(given_type), pos + 1,
                   scamtype_name(req_type));
}

scamval* scamerr_type2(const char* name, size_t pos, int given_type) {
    return scamerr("'%s' got %s as arg %d", name, 
                   scamtype_name(given_type), pos + 1);
}

scamval* scamfunction(scamenv* env, scamval* parameters, scamval* body) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_FUNCTION;
    ret->vals.fun = my_malloc(sizeof *ret->vals.fun);
    ret->vals.fun->env = scamenv_init(env);
    //ret->vals.fun->env = scamenv_copy(env);
    ret->vals.fun->parameters = parameters;
    ret->vals.fun->body = body;
    return ret;
}

scamval* scambuiltin(scambuiltin_t* bltin) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_BUILTIN;
    ret->vals.bltin = bltin;
    return ret;
}

scamval* scamport(FILE* fp) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_PORT;
    ret->vals.port = fp;
    return ret;
}

scamval* scamnull() {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_NULL;
    return ret;
}

scamval* scamval_copy(const scamval* v) {
    scamval* ret = my_malloc(sizeof *ret);
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
            ret->vals.s = my_malloc(strlen(v->vals.s) + 1);
            strcpy(ret->vals.s, v->vals.s);
            break;
        case SCAM_FUNCTION:
            ret->vals.fun = my_malloc(sizeof(scamfun_t));
            ret->vals.fun->env = scamenv_init(v->vals.fun->env);
            ret->vals.fun->parameters = scamval_copy(v->vals.fun->parameters);
            ret->vals.fun->body = scamval_copy(v->vals.fun->body);
            break;
        case SCAM_BUILTIN:
            ret->vals.bltin = v->vals.bltin;
        case SCAM_PORT:
            ret->vals.port = v->vals.port;
    }
    return ret;
}

int is_numeric_type(const scamval* v) {
    return v->type == SCAM_INT || v->type == SCAM_DEC;
}

int scamval_numeric_eq(const scamval* v1, const scamval* v2) {
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

int scamval_list_eq(const scamval* v1, const scamval* v2) {
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

int scamval_eq(const scamval* v1, const scamval* v2) {
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
            case SCAM_NULL:
                return 1;
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
            fclose(v->vals.port);
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
    scamenv* ret = my_malloc(sizeof *ret);
    ret->enclosing = enclosing;
    ret->syms = array_init();
    ret->vals = array_init();
    return ret;
}

void scamenv_bind(scamenv* env, scamval* sym, scamval* val) {
    for (int i = 0; i < array_len(env->syms); i++) {
        if (scamval_eq(array_get(env->syms, i), sym)) {
            // free the symbol and the old value
            scamval_free(sym);
            scamval_free(array_get(env->vals, i));
            array_set(env->vals, i, val);
            return;
        }
    }
    array_append(env->syms, sym);
    array_append(env->vals, val);
}

scamenv* scamenv_copy(const scamenv* env) {
    printf("copying env with %ld bindings\n", array_len(env->syms));
    scamenv* ret = my_malloc(sizeof *ret);
    ret->enclosing = env->enclosing;
    ret->syms = array_copy(env->syms);
    ret->vals = array_copy(env->vals);
    return ret;
}

void scamenv_free(scamenv* env) {
    if (env) {
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
        return scamerr("unbound variable '%s'", name->vals.s);
    }
}

void array_print(const array* arr, char* open, char* close) {
    if (!arr) return;
    printf("%s", open);
    for (int i = 0; i < arr->count; i++) {
        scamval_print(arr->root[i]);
        if (i != arr->count - 1)
            printf(" ");
    }
    printf("%s", close);
}

void scamval_print(const scamval* v) {
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

void scamval_print_debug(const scamval* v) {
    scamval_print(v);
    printf(" (%s)", scamtype_debug_name(v->type));
}

void scamval_println(const scamval* v) {
    if (!v || v->type == SCAM_NULL) return;
    scamval_print(v);
    printf("\n");
}

void scamval_print_ast(const scamval* ast, int indent) {
    for (int i = 0; i < indent; i++)
        printf("  ");
    if (ast->type == SCAM_CODE || ast->type == SCAM_PROGRAM) {
        printf(ast->type == SCAM_CODE ? "EXPR\n" : "PROGRAM\n");
        for (int i = 0; i < scamval_len(ast); i++) {
            scamval_print_ast(scamval_get(ast, i), indent + 1);
        }
    } else {
        scamval_println(ast);
    }
}

const char* scamtype_name(int type) {
    switch (type) {
        case SCAM_INT: return "integer";
        case SCAM_DEC: return "decimal";
        case SCAM_BOOL: return "boolean";
        case SCAM_LIST: return "list";
        case SCAM_STR: return "string";
        case SCAM_QUOTE: return "quote";
        case SCAM_FUNCTION: return "function";
        case SCAM_PORT: return "port";
        case SCAM_BUILTIN: return "function";
        case SCAM_CODE: return "code";
        case SCAM_SYM: return "symbol";
        case SCAM_ERR: return "error";
        case SCAM_NULL: return "null";
        case SCAM_PROGRAM: return "program";
        default: return "bad scamval type";
    }
}

const char* scamtype_debug_name(int type) {
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
        case SCAM_PROGRAM: return "SCAM_PROGRAM";
        default: return "bad scamval type";
    }
}
