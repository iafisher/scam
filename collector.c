#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "progutils.h"

static scamval** scamval_objs = NULL;
static size_t count = 0;
static size_t first_avail = 0;

enum { HEAP_INIT = 512, HEAP_GROW = 2 };

static void gc_mark(scamval* v) {
    if (v != NULL && !v->seen) {
        v->seen = 1;
        switch (v->type) {
            case SCAM_LIST:
            case SCAM_SEXPR:
                for (size_t i = 0; i < scamseq_len(v); i++)
                    gc_mark(scamseq_get(v, i));
                break;
            case SCAM_LAMBDA:
                gc_mark(v->vals.fun->parameters);
                gc_mark(v->vals.fun->body);
                gc_mark(v->vals.fun->env);
                break;
            case SCAM_DICT:
                gc_mark(v->vals.dct->syms);
                gc_mark(v->vals.dct->vals);
                break;
        }
    }
}

static void gc_sweep() {
    for (size_t i = 0; i < count; i++) {
        scamval* v = scamval_objs[i];
        if (v != NULL) {
            if (!v->seen) {
                gc_del_scamval(v);
                scamval_objs[i] = NULL;
                if (i < first_avail) {
                    first_avail = i;
                }
            } else {
                v->seen = 0;
            }
        }
    }
}

void gc_collect() {
    for (size_t i = 0; i < count; i++) {
        scamval* v = scamval_objs[i];
        if (!v->seen && v->is_root)
            gc_mark(v);
    }
    gc_sweep();
}

void gc_unset_root(scamval* v) {
    v->is_root = 0;
}

scamval* gc_new_scamval() {
    if (!scamval_objs) {
        // initialize internal heap for the first time
        count = HEAP_INIT;
        scamval_objs = my_malloc(count * sizeof *scamval_objs);
        for (size_t i = 0; i < count; i++) {
            scamval_objs[i] = NULL;
        }
    } else if (first_avail == count) {
        gc_collect();
        if (first_avail == count) {
            // grow internal heap
            count *= HEAP_GROW;
            scamval_objs = my_realloc(scamval_objs, 
                                      count * sizeof *scamval_objs);
            for (size_t i = first_avail; i < count; i++) {
                scamval_objs[i] = NULL;
            }
        }
    }
    scamval* ret = my_malloc(sizeof *ret);
    ret->seen = 0;
    ret->is_root = 1;
    scamval_objs[first_avail] = ret;
    while (++first_avail < count && scamval_objs[first_avail] != NULL)
        ;
    return ret;
}

void gc_del_scamval(scamval* v) {
    switch (v->type) {
        case SCAM_LIST:
        case SCAM_SEXPR:
            free(v->vals.arr);
            break;
        case SCAM_ERR:
        case SCAM_SYM:
        case SCAM_STR:
            free(v->vals.s);
            break;
        case SCAM_LAMBDA:
            free(v->vals.fun);
            break;
        case SCAM_PORT:
            if (scamport_status(v) == SCAMPORT_OPEN)
                fclose(scam_as_file(v));
            free(v->vals.port);
            break;
        case SCAM_BUILTIN:
            free(v->vals.bltin);
            break;
        case SCAM_DICT:
            free(v->vals.dct);
            break;
    }
    free(v);
}

scamval* gc_copy_scamval(scamval* v) {
    switch (v->type) {
        case SCAM_LIST:
        case SCAM_SEXPR:
        {
            scamval* ret = scamval_new(v->type);
            ret->vals.arr = my_malloc(v->count * sizeof *v->vals.arr);
            ret->count = v->count;
            ret->mem_size = v->count;
            for (int i = 0; i < v->count; i++) {
                ret->vals.arr[i] = gc_copy_scamval(v->vals.arr[i]);
            }
            return ret;
        }
        case SCAM_STR:
        case SCAM_SYM:
        case SCAM_ERR:
        {
            scamval* ret = scamval_new(v->type);
            ret->vals.s = strdup(v->vals.s);
            return ret;
        }
        default:
            return v;
    }
}

void gc_close() {
    for (size_t i = 0; i < count; i++) {
        scamval* v = scamval_objs[i];
        if (v != NULL) {
            gc_del_scamval(v);
        }
    }
    free(scamval_objs);
}
