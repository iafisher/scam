#include "scamval.h"

void scamstr_resize(scamval* s, size_t new_sz) {
    s->mem_size = new_sz;
    if (s->vals.s == NULL) {
        s->vals.s = my_malloc(new_sz);
    } else {
        s->vals.s = my_realloc(s->vals.s, new_sz);
    }
}

size_t scamstr_len(const scamval* s) {
    return s->count;
}

void scamstr_concat(scamval* s1, scamval* s2) {
    size_t n1 = scamstr_len(s1);
    size_t n2 = scamstr_len(s2);
    scamstr_resize(s1, n1 + n2 + 1);
    for (int i = 0; i <= n2; i++) {
        s1->vals.s[n1 + i] = s2->vals.s[i];
    }
    s1->count = n1 + n2;
    scamval_free(s2);
}
