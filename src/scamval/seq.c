#include <stdarg.h>
#include <string.h>
#include "collector.h"
#include "scamval.h"


/* Construct an internal sequence from a variable argument list. */
static ScamSeq* ScamSeq_new_from(int type, size_t n, va_list vlist);

/* Grow the size of the sequence in memory so that it is at least the given minimum size, and
 * possibly larger.
 */
static void ScamSeq_grow(ScamSeq* seq, size_t min_new_sz);
/* Unlike ScamSeq_grow, the new sequence is guaranteed to be exactly the new size provided. */
static void ScamSeq_resize(ScamSeq* seq, size_t new_sz);


/* Construct a value that is internally a sequence. */
static ScamSeq* ScamSeq_new(int type) {
    SCAMVAL_NEW(ret, ScamSeq, type);
    ret->count = 0;
    ret->mem_size = 0;
    ret->arr = NULL;
    return ret;
}


ScamSeq* ScamList_new(void) {
    return ScamSeq_new(SCAM_LIST);
}


ScamSeq* ScamList_from(size_t n, ...) {
    va_list vlist;
    va_start(vlist, n);
    return ScamSeq_new_from(SCAM_LIST, n, vlist);
}


ScamSeq* ScamExpr_new(void) {
    return ScamSeq_new(SCAM_SEXPR);
}


ScamSeq* ScamExpr_from(size_t n, ...) {
    va_list vlist;
    va_start(vlist, n);
    return ScamSeq_new_from(SCAM_SEXPR, n, vlist);
}


ScamVal* ScamSeq_pop(ScamSeq* seq, size_t i) {
    if (i >= 0 && i < seq->count) {
        ScamVal* ret = seq->arr[i];
        memmove(seq->arr+i, seq->arr+i+1, (seq->count-i-1) * sizeof *seq->arr);
        seq->count--;
        gc_set_root(ret);
        return ret;
    } else {
        return (ScamVal*)ScamErr_new("attempted sequence access out of range");
    }
}


void ScamSeq_delete(ScamSeq* seq, size_t i) {
    gc_unset_root(ScamSeq_pop(seq, i));
}


ScamVal* ScamSeq_get(const ScamSeq* seq, size_t i) {
    return seq->arr[i];
}


size_t ScamSeq_len(const ScamSeq* seq) {
    return seq->count;
}


void ScamSeq_set(ScamSeq* seq, size_t i, ScamVal* v) {
    if (i >= 0 && i < seq->count) {
        seq->arr[i] = v;
    }
}


void ScamSeq_prepend(ScamSeq* seq, ScamVal* v) {
    ScamSeq_insert(seq, 0, v);
}


void ScamSeq_append(ScamSeq* seq, ScamVal* v) {
    ScamSeq_insert(seq, ScamSeq_len(seq), v);
}


void ScamSeq_insert(ScamSeq* seq, size_t i, ScamVal* v) {
    gc_unset_root(v);
    if (++seq->count > seq->mem_size) {
        ScamSeq_grow(seq, seq->count);
    }
    memmove(seq->arr+i+1, seq->arr+i, (seq->count-i-1) * sizeof *seq->arr);
    seq->arr[i] = v;
}


void ScamSeq_concat(ScamSeq* seq1, ScamSeq* seq2) {
    if (ScamSeq_len(seq2) > 0) {
        ScamSeq_resize(seq1, ScamSeq_len(seq1) + ScamSeq_len(seq2));
        while (ScamSeq_len(seq2) > 0) {
            ScamSeq_append(seq1, ScamSeq_pop(seq2, 0));
        }
    }
}


ScamVal* ScamSeq_subseq(const ScamSeq* seq, size_t start, size_t end) {
    size_t n = ScamSeq_len(seq);
    if (start >= 0 && end <= n && start <= end) {
        ScamSeq* ret = ScamSeq_new(seq->type);
        for (size_t i = start; i < end; i++) {
            ScamSeq_append(ret, gc_copy_ScamVal(ScamSeq_get(seq, i)));
        }
        return (ScamVal*)ret;
    } else {
        return (ScamVal*)ScamErr_new("attempted sequence access out of bounds");
    }
}


static ScamSeq* ScamSeq_new_from(int type, size_t n, va_list vlist) {
    SCAMVAL_NEW(ret, ScamSeq, type);
    ret->arr = gc_malloc(n * sizeof *ret->arr);
    for (int i = 0; i < n; i++) {
        ret->arr[i] = va_arg(vlist, ScamVal*);
        gc_unset_root(ret->arr[i]);
    }
    ret->count = n;
    ret->mem_size = n;
    va_end(vlist);
    return ret;
}


enum { SEQ_SIZE_INITIAL = 5, SEQ_SIZE_GROW = 2};
static void ScamSeq_grow(ScamSeq* seq, size_t min_new_sz) {
    if (seq->arr == NULL) {
        seq->mem_size = SEQ_SIZE_INITIAL;
        if (seq->mem_size < min_new_sz)
            seq->mem_size = min_new_sz;
        seq->arr = gc_malloc(seq->mem_size * sizeof *seq->arr);
    } else {
        seq->mem_size *= SEQ_SIZE_GROW;
        if (seq->mem_size < min_new_sz)
            seq->mem_size = min_new_sz;
        seq->arr = gc_realloc(seq->arr, seq->mem_size * sizeof *seq->arr);
    }
}


static void ScamSeq_resize(ScamSeq* seq, size_t new_sz) {
    seq->mem_size = new_sz;
    if (seq->arr == NULL) {
        seq->arr = gc_malloc(seq->mem_size * sizeof *seq->arr);
    } else {
        seq->arr = gc_realloc(seq->arr, seq->mem_size * sizeof *seq->arr);
    }
}
