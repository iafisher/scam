#include <stdlib.h>
#include <string.h>
#include "collector.h"

static scamval** scamval_objs = NULL;
static size_t count = 0;
static size_t first_avail = 0;

// if you change either of these, make sure that the scamdict function in scamval.c will still work
enum { HEAP_INIT = 1024, HEAP_GROW = 2 };

// Mark all objects which can be reached from the given object
static void gc_mark(scamval* v) {
    if (v != NULL && !v->seen) {
        v->seen = 1;
        switch (v->type) {
            case SCAM_LIST:
            case SCAM_SEXPR:
                for (size_t i = 0; i < scamseq_len(v); i++) {
                    gc_mark(scamseq_get(v, i));
                }
                break;
            case SCAM_LAMBDA:
                gc_mark(v->vals.fun->parameters);
                gc_mark(v->vals.fun->body);
                gc_mark(v->vals.fun->env);
                break;
            case SCAM_DICT:
                gc_mark(v->vals.dct->enclosing);
                gc_mark(v->vals.dct->syms);
                gc_mark(v->vals.dct->vals);
                break;
        }
    }
}

static void gc_del_scamval(scamval* v) {
    switch (v->type) {
        case SCAM_LIST:
        case SCAM_SEXPR:
        case SCAM_DOT_SYM:
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

// Sweep the entire heap, freeing items that have not been marked and resetting the marks on those 
// that have
static void gc_sweep(void) {
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

void gc_collect(void) {
    for (size_t i = 0; i < count; i++) {
        scamval* v = scamval_objs[i];
        if (v != NULL && !v->seen && v->is_root) {
            gc_mark(v);
        }
    }
    gc_sweep();
}

void gc_unset_root(scamval* v) {
    v->is_root = 0;
}

void gc_set_root(scamval* v) {
    v->is_root = 1;
}

scamval* gc_new_scamval(int type) {
    if (scamval_objs == NULL) {
        // initialize internal heap for the first time
        count = HEAP_INIT;
        scamval_objs = gc_malloc(count * sizeof *scamval_objs);
        for (size_t i = 0; i < count; i++) {
            scamval_objs[i] = NULL;
        }
    } else if (first_avail == count) {
        gc_collect();
        if (first_avail == count) {
            // grow internal heap
            size_t new_count = count * HEAP_GROW;
            scamval_objs = gc_realloc(scamval_objs, 
                                      new_count * sizeof *scamval_objs);
            for (size_t i = count; i < new_count; i++) {
                scamval_objs[i] = NULL;
            }
            count = new_count;
        }
    }
    scamval* ret = gc_malloc(sizeof *ret);
    ret->type = type;
    ret->seen = 0;
    ret->is_root = 1;
    ret->nspace = NULL;
    scamval_objs[first_avail] = ret;
    // update first_avail to be the first available heap location
    while (++first_avail < count && scamval_objs[first_avail] != NULL)
        ;
    return ret;
}

scamval* gc_copy_scamval(scamval* v) {
    switch (v->type) {
        case SCAM_LIST:
        case SCAM_SEXPR:
        {
            scamval* ret = gc_new_scamval(v->type);
            ret->vals.arr = gc_malloc(v->count * sizeof *v->vals.arr);
            ret->count = 0;
            ret->mem_size = v->count;
            for (int i = 0; i < v->count; i++) {
                ret->vals.arr[i] = gc_copy_scamval(v->vals.arr[i]);
                gc_unset_root(ret->vals.arr[i]);
                // count must always contain an accurate count of the allocated elements of the 
                // list, in case the garbage collector is invoked in the middle of copying and needs
                // to mark the elements of this list
                ret->count++;
            }
            return ret;
        }
        case SCAM_STR:
            return scamstr(scam_as_str(v));
        case SCAM_SYM:
            return scamsym(scam_as_str(v));
        case SCAM_ERR:
            return scamerr(scam_as_str(v));
        case SCAM_DICT:
        {
            scamval* ret = scamdict(scamdict_enclosing(v));
            scamdict_set_keys(ret, gc_copy_scamval(scamdict_keys(v)));
            scamdict_set_vals(ret, gc_copy_scamval(scamdict_vals(v)));
            return ret;
        }
        default:
            v->is_root = 1;
            return v;
    }
}

void gc_close(void) {
    for (size_t i = 0; i < count; i++) {
        scamval* v = scamval_objs[i];
        if (v != NULL) {
            gc_del_scamval(v);
        }
    }
    free(scamval_objs);
}

void gc_print(void) {
    printf("Allocated space for %ld references\n", count);
    for (size_t i = 0; i < count; i++) {
        scamval* v = scamval_objs[i];
        if (v != NULL) {
            printf("%.4ld: ", i);
            scamval_print_debug(v);
            if (v->is_root)
                printf(" (root)");
            printf("\n");
        }
    }
}

static size_t first_interesting_index(void) {
    // first 3 refs are for the global environment
    int reached_the_builtin_ports = 0;
    for (size_t i = 3; i < count; i++) {
        scamval* v = scamval_objs[i];
        if (v == NULL) {
            return i;
        } 
        // even indices should be builtins
        if (i % 2 == 0) {
            if (v->type != SCAM_SYM) {
                return i;
            } else if (reached_the_builtin_ports) {
                if (strcmp(scam_as_str(v), "stdout") != 0 &&
                    strcmp(scam_as_str(v), "stdin")  != 0 &&
                    strcmp(scam_as_str(v), "stderr") != 0) {
                    return i - 1;
                }
            }
        // odd indices should be symbols
        } else {
            if (v->type == SCAM_PORT) {
                reached_the_builtin_ports = 1;
            } else if (v->type != SCAM_BUILTIN) {
                return i;
            }
        }
    }
    return count;
}

void gc_smart_print(void) {
    printf("Allocated space for %ld references\n", count);
    for (size_t i = first_interesting_index(); i < count; i++) {
        scamval* v = scamval_objs[i];
        if (v != NULL) {
            printf("%.4ld: ", i);
            scamval_print_debug(v);
            if (v->is_root)
                printf(" (root)");
            printf("\n");
        }
    }
}

void* gc_malloc(size_t size) {
    void* ret = malloc(size);
    if (ret == NULL) {
        gc_collect();
        ret = malloc(size);
        if (ret == NULL) {
            fputs("out of memory... exiting program\n", stderr);
            exit(EXIT_FAILURE);
        }
    }
    return ret;
}

void* gc_realloc(void* ptr, size_t size) {
    void* ret = realloc(ptr, size);
    if (ret == NULL) {
        gc_collect();
        ret = realloc(ptr, size);
        if (ret == NULL) {
            fputs("out of memory... exiting program\n", stderr);
            exit(EXIT_FAILURE);
        }
    }
    return ret;
}

void* gc_calloc(size_t num, size_t size) {
    void* ret = calloc(num, size);
    if (ret == NULL) {
        gc_collect();
        ret = calloc(num, size);
        if (ret == NULL) {
            fputs("out of memory... exiting program\n", stderr);
            exit(EXIT_FAILURE);
        }
    }
    return ret;
}
