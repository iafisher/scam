#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "scamval.h"


/* Construct a value that is internally a string (strings, symbols and errors). */
static ScamStr* ScamStr_base_new(int type, const char* s);
static void ScamStr_resize(ScamStr* sbox, size_t new_sz);


ScamStr* ScamStr_new(const char* s) {
    return ScamStr_base_new(SCAM_STR, s);
}


ScamStr* ScamStr_from_literal(char* s) {
    size_t n = strlen(s);
    /* Remove the quotes. */
    s[n - 1] = '\0';
    memmove(s, s + 1, n);
    n--;
    /* Fix the backslash escapes. */
    for (size_t i = 0; i < n; i++) {
        if (s[i] == '\\') {
            switch (s[i + 1]) {
                #define ESCAPE(c, escaped) \
                case escaped: s[i+1] = c; break;
                #define OPTIONAL_ESCAPE(c, escaped) \
                case escaped: s[i+1] = c; break;
                #include "../escape.def"
                default: free(s); return ScamErr_new("invalid backslash escape");
            }
            memmove(s+i, s+i+1, n-i);
            n--;
        }
    }
    return ScamStr_no_copy(s);
}


ScamStr* ScamStr_read(FILE* fp) {
    SCAMVAL_NEW(ret, ScamStr, SCAM_STR);
    ret->s = NULL;
    ret->count = getline(&ret->s, &ret->mem_size, fp);
    if (ret->count != -1) {
        return ret;
    } else {
        gc_unset_root((ScamVal*)ret);
        return (ScamStr*)ScamErr_eof();
    }
}


ScamStr* ScamStr_empty(void) {
    SCAMVAL_NEW(ret, ScamStr, SCAM_STR);
    ret->count = 0;
    ret->mem_size = 0;
    ret->s = NULL;
    return ret;
}


ScamStr* ScamStr_no_copy(char* s) {
    SCAMVAL_NEW(ret, ScamStr, SCAM_STR);
    ret->s = s;
    ret->count = ret->mem_size = strlen(s);
    return ret;
}


ScamStr* ScamStr_from_char(char c) {
    SCAMVAL_NEW(ret, ScamStr, SCAM_STR);
    ret->s = gc_malloc(2);
    ret->s[0] = c;
    ret->s[1] = '\0';
    ret->count = 1;
    ret->mem_size = 2;
    return ret;
}


ScamStr* ScamSym_new(const char* s) {
    return (ScamStr*)ScamStr_base_new(SCAM_SYM, s);
}


ScamStr* ScamSym_no_copy(char* s) {
    SCAMVAL_NEW(ret, ScamStr, SCAM_SYM);
    ret->s = s;
    ret->count = ret->mem_size = strlen(s);
    return ret;
}


enum { MAX_ERROR_SIZE = 100 };
ScamStr* ScamErr_new(const char* format, ...) {
    SCAMVAL_NEW(ret, ScamStr, SCAM_ERR);
    va_list vlist;
    va_start(vlist, format);
    ret->s = gc_malloc(MAX_ERROR_SIZE);
    vsnprintf(ret->s, MAX_ERROR_SIZE, format, vlist);
    va_end(vlist);
    return ret;
}


ScamStr* ScamErr_arity(const char* name, size_t got, size_t expected) {
    return ScamErr_new("'%s' got %d arg(s), expected %d", name, got, expected);
}


ScamStr* ScamErr_min_arity(const char* name, size_t got, size_t expected) {
    return ScamErr_new("'%s' got %d arg(s), expected at least %d", name, got, expected);
}


ScamStr* ScamErr_eof(void) {
    return ScamErr_new("reached EOF while reading from a port");
}


const char* ScamStr_unbox(const ScamStr* sbox) {
    return sbox->s;
}


void ScamStr_set(ScamStr* sbox, size_t i, char c) {
    if (i >= 0 && i < sbox->count) {
        sbox->s[i] = c;
    }
}


void ScamStr_map(ScamStr* sbox, int map_f(int)) {
    for (char* p = sbox->s; *p != '\0'; p++) {
        *p = map_f(*p);
    }
}

char ScamStr_get(const ScamStr* sbox, size_t i) {
    if (i >= 0 && i < ScamStr_len(sbox)) {
        return sbox->s[i];
    } else {
        return EOF;
    }
}


char ScamStr_pop(ScamStr* sbox, size_t i) {
    if (i >= 0 && i < ScamStr_len(sbox)) {
        char ret = sbox->s[i];
        ScamStr_remove(sbox, i, i+1);
        return ret;
    } else {
        return EOF;
    }
}


void ScamStr_remove(ScamStr* sbox, size_t start, size_t end) {
    if (start >= 0 && end <= ScamStr_len(sbox) && start < end) {
        memmove(sbox->s+start, sbox->s+end, sbox->count-end);
        sbox->count -= (end - start);
        sbox->s[sbox->count] = '\0';
    }
}


void ScamStr_truncate(ScamStr* sbox, size_t i) {
    if (i >= 0 && i < ScamStr_len(sbox)) {
        sbox->s[i] = '\0';
    }
}


ScamStr* ScamStr_substr(const ScamStr* sbox, size_t start, size_t end) {
    if (start >= 0 && end <= ScamStr_len(sbox) && start <= end) {
        char* s = gc_malloc(end - start + 1);
        strncpy(s, sbox->s+start, end-start);
        s[end - start] = '\0';
        return ScamStr_no_copy(s);
    } else {
        return (ScamStr*)ScamErr_new("string access out of bounds");
    }
}


size_t ScamStr_len(const ScamStr* sbox) {
    return sbox->count;
}


void ScamStr_concat(ScamStr* s1, ScamStr* s2) {
    size_t n1 = ScamStr_len(s1);
    size_t n2 = ScamStr_len(s2);
    ScamStr_resize(s1, n1+n2+1);
    for (int i = 0; i <= n2; i++) {
        s1->s[n1 + i] = s2->s[i];
    }
    s1->count = n1 + n2;
    gc_unset_root((ScamVal*)s2);
}


static ScamStr* ScamStr_base_new(int type, const char* s) {
    SCAMVAL_NEW(ret, ScamStr, type);
    ret->count = strlen(s);
    ret->mem_size = ret->count + 1;
    ret->s = strdup(s);
    return ret;
}


static void ScamStr_resize(ScamStr* sbox, size_t new_sz) {
    sbox->mem_size = new_sz;
    if (sbox->s == NULL) {
        sbox->s = gc_malloc(new_sz);
    } else {
        sbox->s = gc_realloc(sbox->s, new_sz);
    }
}
