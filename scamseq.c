#include <stdlib.h>
#include <string.h>
#include "scamval.h"

// constants for the scamseq_grow function
enum { SEQ_SIZE_INITIAL = 5, SEQ_SIZE_GROW = 2};

// Grow the size of the sequence in memory so that it is at least the given
// minimum size, and possibly larger
void scamseq_grow(scamval* seq, size_t min_new_sz) {
    if (seq->vals.arr == NULL) {
        seq->mem_size = SEQ_SIZE_INITIAL;
        if (seq->mem_size < min_new_sz)
            seq->mem_size = min_new_sz;
        seq->vals.arr = my_malloc(seq->mem_size * sizeof *seq->vals.arr);
    } else {
        seq->mem_size *= SEQ_SIZE_GROW;
        if (seq->mem_size < min_new_sz)
            seq->mem_size = min_new_sz;
        seq->vals.arr = my_realloc(seq->vals.arr, 
                                   seq->mem_size * sizeof *seq->vals.arr);
    }
}

// Unlike scamseq_grow, the new sequence is guaranteed to be exactly the new
// size provided
void scamseq_resize(scamval* seq, size_t new_sz) {
    seq->mem_size = new_sz;
    if (seq->vals.arr == NULL) {
        seq->vals.arr = my_malloc(seq->mem_size * sizeof *seq->vals.arr);
    } else {
        seq->vals.arr = my_realloc(seq->vals.arr,
                                   seq->mem_size * sizeof *seq->vals.arr);
    }
}

scamval* scamseq_pop(scamval* seq, size_t i) {
    if (i >= 0 && i < seq->count) {
        scamval* ret = seq->vals.arr[i];
        memmove(seq->vals.arr + i, seq->vals.arr + i + 1,
                (seq->count - i - 1) * sizeof *seq->vals.arr);
        seq->count--;
        return ret;
    } else {
        return scamerr("attempted sequence access out of range");
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

size_t scamseq_len(const scamval* seq) {
    return seq->count;
}

void scamseq_set(scamval* seq, size_t i, scamval* v) {
    if (i >= 0 && i < seq->count) {
        seq->vals.arr[i] = v;
    }
}

void scamseq_prepend(scamval* seq, scamval* v) {
    size_t new_sz = seq->count + 1;
    if (new_sz > seq->mem_size) {
        scamseq_grow(seq, new_sz);
    }
    seq->count = new_sz;
    memmove(seq->vals.arr + 1, seq->vals.arr, 
            (seq->count - 1) * sizeof *seq->vals.arr);
    seq->vals.arr[0] = v;
}

void scamseq_append(scamval* seq, scamval* v) {
    size_t new_sz = seq->count + 1;
    if (new_sz > seq->mem_size) {
        scamseq_grow(seq, new_sz);
    }
    seq->count = new_sz;
    seq->vals.arr[new_sz - 1] = v;
}

void scamseq_concat(scamval* seq1, scamval* seq2) {
    scamseq_resize(seq1, scamseq_len(seq1) + scamseq_len(seq2));
    while (scamseq_len(seq2) > 0) {
        scamseq_append(seq1, scamseq_pop(seq2, 0));
    }
    scamval_free(seq2);
}

void scamseq_free(scamval* seq) {
    if (seq->vals.arr) {
        for (int i = 0; i < seq->count; i++) {
            scamval_free(seq->vals.arr[i]);
        }
        free(seq->vals.arr);
    }
}
