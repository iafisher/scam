#include <stdlib.h>
#include <string.h>
#include "collector.h"


static ScamVal** scamval_objs = NULL;
static size_t count = 0;
static size_t first_avail = 0;


/* If you change either of these, make sure that the ScamDict_new function in dict.c will still
 * work.
 */
enum { HEAP_INIT = 1024, HEAP_GROW = 2 };
static void gc_init();


/* Mark all objects which can be reached from the given object. */
static void gc_mark(ScamVal* v) {
    if (v != NULL && !v->seen) {
        v->seen = true;
        switch (v->type) {
            case SCAM_LIST:
            case SCAM_SEXPR:
                for (size_t i = 0; i < ScamSeq_len((ScamSeq*)v); i++) {
                    gc_mark(ScamSeq_get((ScamSeq*)v, i));
                }
                break;
            case SCAM_FUNCTION:
                {
                    ScamFunction* f = (ScamFunction*)v;
                    gc_mark((ScamVal*)(f->parameters));
                    gc_mark((ScamVal*)(f->body));
                    gc_mark((ScamVal*)(f->env));
                }
                break;
            case SCAM_DICT:
                {
                    ScamDict* dct = (ScamDict*)v;
                    gc_mark((ScamVal*)(dct->enclosing));
                    for (size_t i = 0; i < SCAM_DICT_SIZE; i++) {
                        for (ScamDict_list* p = dct->data[i]; p != NULL; p = p->next) {
                            gc_mark(p->key);
                            gc_mark(p->val);
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
}


static void gc_del_ScamVal(ScamVal* v) {
    switch (v->type) {
        case SCAM_LIST:
        case SCAM_SEXPR:
            free(((ScamSeq*)v)->arr);
            break;
        case SCAM_ERR:
        case SCAM_SYM:
        case SCAM_STR:
            free(((ScamStr*)v)->s);
            break;
        case SCAM_PORT:
            if (ScamPort_status((ScamPort*)v) == SCAMPORT_OPEN)
                fclose(ScamPort_unbox((ScamPort*)v));
            break;
        case SCAM_DICT:
            for (size_t i = 0; i < SCAM_DICT_SIZE; i++) {
                ScamDict_list_free(((ScamDict*)v)->data[i]);
            }
            break;
        default:
            break;
    }
    free(v);
}


/* Sweep the entire heap, freeing items that have not been marked and resetting the marks on those
 * that have.
 */
static void gc_sweep(void) {
    for (size_t i = 0; i < count; i++) {
        ScamVal* v = scamval_objs[i];
        if (v != NULL) {
            if (!v->seen) {
                gc_del_ScamVal(v);
                scamval_objs[i] = NULL;
                if (i < first_avail) {
                    first_avail = i;
                }
            } else {
                v->seen = false;
            }
        }
    }
}


void gc_collect(void) {
    for (size_t i = 0; i < count; i++) {
        ScamVal* v = scamval_objs[i];
        if (v != NULL && !v->seen && v->is_root) {
            gc_mark(v);
        }
    }
    gc_sweep();
}


void gc_unset_root(ScamVal* v) {
    v->is_root = false;
}


void gc_set_root(ScamVal* v) {
    v->is_root = true;
}


ScamVal* gc_new_ScamVal(int type, size_t sz) {
    if (scamval_objs == NULL) {
        // initialize internal heap for the first time
        gc_init();
    } else if (first_avail == count) {
        gc_collect();
        if (first_avail == count) {
            // grow internal heap
            size_t new_count = count * HEAP_GROW;
            scamval_objs = gc_realloc(scamval_objs, new_count*sizeof *scamval_objs);
            for (size_t i = count; i < new_count; i++) {
                scamval_objs[i] = NULL;
            }
            count = new_count;
        }
    }
    ScamVal* ret = gc_malloc(sz);
    ret->type = type;
    ret->seen = false;
    ret->is_root = true;
    scamval_objs[first_avail] = ret;
    // update first_avail to be the first available heap location
    while (++first_avail < count && scamval_objs[first_avail] != NULL)
        ;
    return ret;
}


ScamVal* gc_copy_ScamVal(ScamVal* v) {
    switch (v->type) {
        case SCAM_LIST:
        case SCAM_SEXPR:
        {
            ScamSeq* seq = (ScamSeq*)v;
            SCAMVAL_NEW(ret, ScamSeq, seq->type);
            ret->arr = gc_malloc(seq->count * sizeof *seq->arr);
            ret->count = 0;
            ret->mem_size = seq->count;
            for (int i = 0; i < seq->count; i++) {
                ret->arr[i] = gc_copy_ScamVal(seq->arr[i]);
                gc_unset_root(ret->arr[i]);
                /* count must always contain an accurate count of the allocated elements of the
                 * list, in case the garbage collector is invoked in the middle of copying and needs
                 * to mark the elements of this list.
                 */
                ret->count++;
            }
            return (ScamVal*)ret;
        }
        case SCAM_STR:
            return (ScamVal*)ScamStr_new(ScamStr_unbox((ScamStr*)v));
        case SCAM_SYM:
            return (ScamVal*)ScamSym_new(ScamStr_unbox((ScamStr*)v));
        case SCAM_ERR:
            return (ScamVal*)ScamErr_new(ScamStr_unbox((ScamStr*)v));
        case SCAM_DICT:
        {
            ScamDict* dct = (ScamDict*)v;
            ScamDict* ret = ScamDict_new(ScamDict_enclosing(dct));
            for (size_t i = 0; i < SCAM_DICT_SIZE; i++) {
                for (ScamDict_list* p = dct->data[i]; p != NULL; p = p->next) {
                    ScamDict_bind(ret, p->key, p->val);
                }
            }
            return (ScamVal*)ret;
        }
        default:
            v->is_root = true;
            return v;
    }
}


void gc_close(void) {
    for (size_t i = 0; i < count; i++) {
        ScamVal* v = scamval_objs[i];
        if (v != NULL) {
            gc_del_ScamVal(v);
        }
    }
    free(scamval_objs);
}


void gc_print(void) {
    printf("Allocated space for %ld references\n", count);
    for (size_t i = 0; i < count; i++) {
        ScamVal* v = scamval_objs[i];
        if (v != NULL) {
            printf("%.4ld: ", i);
            ScamVal_print_debug(v);
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
        ScamVal* v = scamval_objs[i];
        if (v == NULL) {
            return i;
        }
        /* Even indices should be builtins. */
        if (i % 2 == 0) {
            if (v->type != SCAM_SYM) {
                return i;
            } else if (reached_the_builtin_ports) {
                if (strcmp(ScamStr_unbox((ScamStr*)v), "stdout") != 0 &&
                    strcmp(ScamStr_unbox((ScamStr*)v), "stdin")  != 0 &&
                    strcmp(ScamStr_unbox((ScamStr*)v), "stderr") != 0) {
                    return i - 1;
                }
            }
        /* Odd indices should be symbols. */
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
        ScamVal* v = scamval_objs[i];
        if (v != NULL) {
            printf("%.4ld: ", i);
            ScamVal_print_debug(v);
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


static void gc_init() {
    count = HEAP_INIT;
    scamval_objs = gc_malloc(count * sizeof *scamval_objs);
    for (size_t i = 0; i < count; i++) {
        scamval_objs[i] = NULL;
    }
}
