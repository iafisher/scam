#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scamval.h"

#define MAX_ERROR_SIZE 100
#define SEQ_SIZE_INITIAL 5
#define SEQ_SIZE_INCREMENT 5

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

void scamseq_free(scamval* seq) {
    if (seq->vals.arr) {
        for (int i = 0; i < seq->count; i++) {
            scamval_free(seq->vals.arr[i]);
        }
        free(seq->vals.arr);
    }
}

void scamseq_grow(scamval* seq, size_t min_new_sz) {
    if (seq->vals.arr == NULL) {
        seq->mem_size = SEQ_SIZE_INITIAL;
        if (seq->mem_size < min_new_sz)
            seq->mem_size = min_new_sz;
        seq->vals.arr = my_malloc(seq->mem_size * sizeof *seq->vals.arr);
    } else {
        // update the memory size and make sure it is at least big enough as
        // requested
        seq->mem_size += SEQ_SIZE_INCREMENT;
        if (seq->mem_size < min_new_sz)
            seq->mem_size = min_new_sz;
        seq->vals.arr = my_realloc(seq->vals.arr, 
                                   seq->mem_size * sizeof *seq->vals.arr);
    }
}

void scamseq_append(scamval* seq, scamval* v) {
    size_t new_sz = seq->count + 1;
    if (new_sz > seq->mem_size) {
        scamseq_grow(seq, new_sz);
    }
    seq->count = new_sz;
    seq->vals.arr[new_sz - 1] = v;
}

void scamseq_prepend(scamval* seq, scamval* v) {
    size_t new_sz = seq->count + 1;
    if (new_sz > seq->mem_size) {
        scamseq_grow(seq, new_sz);
    }
    seq->count = new_sz;
    for (size_t i = new_sz - 1; i > 0; i--) {
        seq->vals.arr[i] = seq->vals.arr[i - 1];
    }
    seq->vals.arr[0] = v;
}

size_t scamseq_len(const scamval* seq) {
    return seq->count;
}

void scamseq_set(scamval* seq, size_t i, scamval* v) {
    if (i >= 0 && i < seq->count) {
        seq->vals.arr[i] = v;
    }
}

void scamseq_replace(scamval* seq, size_t i, scamval* v) {
    scamval_free(scamseq_get(seq, i));
    scamseq_set(seq, i, v);
}

scamval* scamseq_get(const scamval* seq, size_t i) {
    if (i >= 0 && i < seq->count) {
        return seq->vals.arr[i];
    } else {
        return scamerr("attempted sequence access out of range");
    }
}

scamval* scamseq_pop(scamval* seq, size_t i) {
    if (i >= 0 && i < seq->count) {
        scamval* ret = seq->vals.arr[i];
        for (size_t j = i; j < seq->count - 1; j++) {
            seq->vals.arr[j] = seq->vals.arr[j + 1];
        }
        seq->count--;
        return ret;
    } else {
        return scamerr("attempted sequence access out of range");
    }
}

size_t scamstr_len(const scamval* s) {
    return strlen(s->vals.s);
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

// Make a scamval that is internally a sequence (lists, quotes and code)
scamval* scam_internal_seq(int type) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = type;
    ret->count = 0;
    ret->mem_size = 0;
    ret->vals.arr = NULL;
    return ret;
}

scamval* scamlist() {
    return scam_internal_seq(SCAM_LIST);
}

scamval* scamcode() {
    return scam_internal_seq(SCAM_SEXPR);
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

scamval* scamstr_n(const char* s, size_t n) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_STR;
    ret->vals.s = my_malloc(n + 1);
    strncpy(ret->vals.s, s, n);
    ret->vals.s[n] = '\0';
    return ret;
}

scamval* scamstr_from_char(char c) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_STR;
    ret->vals.s = my_malloc(2);
    ret->vals.s[0] = c;
    ret->vals.s[1] = '\0';
    return ret;
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
    ret->vals.fun->env = env;
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
        case SCAM_SEXPR:
            ret->vals.arr = my_malloc(v->count * sizeof *v->vals.arr);
            ret->count = v->count;
            ret->mem_size = v->count;
            for (int i = 0; i < v->count; i++) {
                ret->vals.arr[i] = scamval_copy(v->vals.arr[i]);
            }
            break;
        case SCAM_STR:
        case SCAM_SYM:
        case SCAM_ERR:
            ret->vals.s = my_malloc(strlen(v->vals.s) + 1);
            strcpy(ret->vals.s, v->vals.s);
            break;
        case SCAM_FUNCTION:
            ret->vals.fun = my_malloc(sizeof(scamfun_t));
            ret->vals.fun->env = scamenv_init(v->vals.fun->env);
            ret->vals.fun->env->type = SCAMENV_TMP;
            //ret->vals.fun->env = v->vals.fun->env;
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
    size_t n1 = scamseq_len(v1);
    size_t n2 = scamseq_len(v2);
    if (n1 == n2) {
        for (int i = 0; i < n1; i++) {
            if (!scamval_eq(scamseq_get(v1, i), scamseq_get(v2, i))) {
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
            case SCAM_SEXPR:
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
        case SCAM_SEXPR:
            scamseq_free(v); break;
        case SCAM_STR:
        case SCAM_SYM:
        case SCAM_ERR:
            if (v->vals.s) free(v->vals.s); break;
        case SCAM_PORT:
            fclose(v->vals.port);
            break;
        case SCAM_FUNCTION:
            if (v->vals.fun->env->type == SCAMENV_TMP)
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
    ret->type = SCAMENV_NORMAL;
    ret->enclosing = enclosing;
    ret->syms = scamlist();
    ret->vals = scamlist();
    return ret;
}

// If v is a function, change its environment's type from CLOSURE to TMP
// This allows functions to be properly free'd
void scamenv_set_func_type(scamval* v) {
    if (v->type == SCAM_FUNCTION && v->vals.fun->env) {
        if (v->vals.fun->env->type == SCAMENV_CLOSURE) {
            v->vals.fun->env->type = SCAMENV_TMP;
        }
    }
}

void scamenv_bind(scamenv* env, scamval* sym, scamval* val) {
    for (int i = 0; i < scamseq_len(env->syms); i++) {
        if (scamval_eq(scamseq_get(env->syms, i), sym)) {
            // free the symbol and the old value
            scamval_free(sym);
            scamval* v = scamseq_get(env->vals, i);
            scamenv_set_func_type(v);
            scamval_free(v);
            scamseq_set(env->vals, i, val);
            return;
        }
    }
    scamseq_append(env->syms, sym);
    scamseq_append(env->vals, val);
}

void scamenv_free(scamenv* env) {
    if (env) {
        for (int i = 0; i < scamseq_len(env->vals); i++) {
            scamenv_set_func_type(scamseq_get(env->vals, i));
        }
        scamval_free(env->syms);
        scamval_free(env->vals);
        free(env);
    }
}

scamval* scamenv_lookup(scamenv* env, scamval* name) {
    for (int i = 0; i < scamseq_len(env->syms); i++) {
        if (strcmp(scamseq_get(env->syms, i)->vals.s, name->vals.s) == 0) {
            return scamval_copy(scamseq_get(env->vals, i));
        }
    }
    if (env->enclosing != NULL) {
        return scamenv_lookup(env->enclosing, name);
    } else {
        return scamerr("unbound variable '%s'", name->vals.s);
    }
}

void scamseq_print(const scamval* seq, char* open, char* close) {
    printf("%s", open);
    for (int i = 0; i < seq->count; i++) {
        scamval_print(seq->vals.arr[i]);
        if (i != seq->count - 1)
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
        case SCAM_LIST: scamseq_print(v, "[", "]"); break;
        case SCAM_SEXPR: scamseq_print(v, "(", ")"); break;
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
    if (ast->type == SCAM_SEXPR) {
        printf("EXPR\n");
        for (int i = 0; i < scamseq_len(ast); i++) {
            scamval_print_ast(scamseq_get(ast, i), indent + 1);
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
        case SCAM_FUNCTION: return "function";
        case SCAM_PORT: return "port";
        case SCAM_BUILTIN: return "function";
        case SCAM_SEXPR: return "S-expression";
        case SCAM_SYM: return "symbol";
        case SCAM_ERR: return "error";
        case SCAM_NULL: return "null";
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
        case SCAM_FUNCTION: return "SCAM_FUNCTION";
        case SCAM_PORT: return "SCAM_PORT";
        case SCAM_BUILTIN: return "SCAM_BUILTIN";
        case SCAM_SEXPR: return "SCAM_SEXPR";
        case SCAM_SYM: return "SCAM_SYM";
        case SCAM_ERR: return "SCAM_ERR";
        case SCAM_NULL: return "SCAM_NULL";
        default: return "bad scamval type";
    }
}
