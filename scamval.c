#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scamval.h"
#include "scamtypes.h"

#define MAX_ERROR_SIZE 100

void* my_malloc(size_t size) {
    void* ret = malloc(size);
    if (ret == NULL) {
        fputs("malloc returned a NULL pointer... exiting program\n", stderr);
        exit(EXIT_FAILURE);
    }
    return ret;
}

void* my_realloc(void* ptr, size_t size) {
    void* ret = realloc(ptr, size);
    if (ret == NULL) {
        fputs("realloc returned a NULL pointer... exiting program", stderr);
        exit(EXIT_FAILURE);
    }
    return ret;
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

scamval* scamsexpr() {
    return scam_internal_seq(SCAM_SEXPR);
}

scamval* scamsexpr_from_vals(size_t n, ...) {
    va_list vlist;
    va_start(vlist, n);
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_SEXPR;
    ret->count = n;
    ret->mem_size = n;
    ret->vals.arr = my_malloc(n * sizeof *ret->vals.arr);
    for (int i = 0; i < n; i++) {
        scamval* v = va_arg(vlist, scamval*);
        ret->vals.arr[i] = v;
    }
    va_end(vlist);
    return ret;
}

// Make a scamval that is internally a string (strings, symbols and errors)
scamval* scam_internal_str(int type, const char* s) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = type;
    ret->count = strlen(s);
    ret->mem_size = ret->count + 1;
    ret->vals.s = strdup(s);
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
    ret->count = n;
    ret->mem_size = n + 1;
    return ret;
}

scamval* scamstr_from_char(char c) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_STR;
    ret->vals.s = my_malloc(2);
    ret->vals.s[0] = c;
    ret->vals.s[1] = '\0';
    ret->count = 1;
    ret->mem_size = 2;
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

scamval* scamfunction(scamenv* env, scamval* parameters, scamval* body) {
    scamval* ret = my_malloc(sizeof *ret);
    ret->type = SCAM_FUNCTION;
    ret->vals.fun = my_malloc(sizeof *ret->vals.fun);
    ret->vals.fun->env = env;
    env->references++;
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
    ret->line = v->line;
    ret->col = v->col;
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
            ret->vals.s = strdup(v->vals.s);
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
    if (scamval_typecheck(v1, SCAM_NUM) && scamval_typecheck(v2, SCAM_NUM)) {
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

int scamval_numeric_gt(const scamval* v1, const scamval* v2) {
    if (v1->type == SCAM_INT) {
        if (v2->type == SCAM_INT) {
            return v1->vals.n > v2->vals.n;
        } else {
            return v1->vals.n > v2->vals.d;
        }
    } else {
        if (v2->type == SCAM_INT) {
            return v1->vals.d > v2->vals.n;
        } else {
            return v1->vals.d > v2->vals.d;
        }
    }
}

int scamval_gt(const scamval* v1, const scamval* v2) {
    if (scamval_typecheck(v1, SCAM_NUM) && scamval_typecheck(v2, SCAM_NUM)) {
        return scamval_numeric_gt(v1, v2);
    } else if (scamval_typecheck(v1, SCAM_STR) && 
               scamval_typecheck(v2, SCAM_STR)) {
        return strcmp(v1->vals.s, v2->vals.s) > 0;
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
            v->vals.fun->env->references--;
            if (v->vals.fun->env->references == 0) {
                scamenv_free(v->vals.fun->env);
            } else if (v->vals.fun->env->references < 0) {
                printf("negative references... something is wrong here\n");
                scamenv_free(v->vals.fun->env);
            }
            scamval_free(v->vals.fun->parameters);
            scamval_free(v->vals.fun->body);
            free(v->vals.fun);
            break;
    }
    free(v);
}

scamenv* scamenv_init(scamenv* enclosing) {
    scamenv* ret = my_malloc(sizeof *ret);
    ret->references = 1;
    ret->enclosing = enclosing;
    ret->syms = scamlist();
    ret->vals = scamlist();
    return ret;
}

void scamenv_bind(scamenv* env, scamval* sym, scamval* val) {
    for (int i = 0; i < scamseq_len(env->syms); i++) {
        if (scamval_eq(scamseq_get(env->syms, i), sym)) {
            // free the symbol and the old value
            scamval_free(sym);
            scamval_free(scamseq_get(env->vals, i));
            scamseq_set(env->vals, i, val);
            return;
        }
    }
    scamseq_append(env->syms, sym);
    scamseq_append(env->vals, val);
}

void scamenv_reset_refs(scamenv* env) {
    env->references = 0;
    env->already_seen = 1;
    for (int i = 0; i < scamseq_len(env->vals); i++) {
        scamval* v = scamseq_get(env->vals, i);
        if (v->type == SCAM_FUNCTION) {
            scamenv_reset_refs(v->vals.fun->env);
        }
    }
}

void scamenv_mark(scamenv* env) {
    env->enclosing->references++;
    for (int i = 0; i < scamseq_len(env->vals); i++) {
        scamval* v = scamseq_get(env->vals, i);
        if (v->type == SCAM_FUNCTION) {
            scamenv_mark(v->vals.fun->env);
        }
    }
}

void scamenv_free(scamenv* env) {
    if (env) {
        /*
        scamenv_reset_refs(env);
        scamenv_mark(env);
        */
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
