#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include "collector.h"
#include "scamval.h"


static unsigned long long hash_int(long long x);
static unsigned long long hash_str(const char* str);
static unsigned long long hash(const ScamVal* v);
static ScamDict_list* ScamDict_list_new(ScamDict_list* next, ScamVal* key, ScamVal* val);


ScamDict* ScamDict_new() {
    SCAMVAL_NEW(ret, ScamDict, SCAM_DICT);
    ret->len = 0;
    for (size_t i = 0; i < SCAM_DICT_SIZE; i++) {
        ret->data[i] = NULL;
    }
    return ret;
}


ScamEnv* ScamEnv_new(ScamEnv* enclosing) {
    SCAMVAL_NEW(ret, ScamEnv, SCAM_ENV);
    ret->enclosing = enclosing;
    ret->len = 0;
    for (size_t i = 0; i < SCAM_DICT_SIZE; i++) {
        ret->data[i] = NULL;
    }
    return ret;
}


ScamDict* ScamDict_from(size_t n, ...) {
    va_list vlist;
    va_start(vlist, n);
    ScamDict* ret = ScamDict_new(NULL);
    for (int i = 0; i < n; i++) {
        ScamSeq* key_val_pair = (ScamSeq*)va_arg(vlist, ScamVal*);
        if (key_val_pair->type == SCAM_LIST && ScamSeq_len(key_val_pair) == 2) {
            ScamVal* key = ScamSeq_get(key_val_pair, 0);
            ScamVal* val = ScamSeq_get(key_val_pair, 1);
            ScamDict_insert(ret, key, val);
        } else {
            gc_unset_root((ScamVal*)ret);
            return (ScamDict*)ScamErr_new("ScamDict_from takes key-value pairs as arguments");
        }
    }
    va_end(vlist);
    return ret;
}


ScamEnv* ScamEnv_enclosing(const ScamEnv* env) {
    return env->enclosing;
}


size_t ScamDict_len(const ScamDict* dct) {
    return dct->len;
}


void ScamDict_insert(ScamDict* dct, ScamVal* sym, ScamVal* val) {
    if (sym->type != SCAM_STR && sym->type != SCAM_SYM && sym->type != SCAM_INT) {
        /* Unbindable types (for now) */
        return;
        /*return ScamErr_new("cannot bind type '%s'", scamtype_name(sym->type));*/
    }
    size_t hashval = hash(sym) % SCAM_DICT_SIZE;
    ScamDict_list* head = dct->data[hashval];
    for (ScamDict_list* p = head; p != NULL; p = p->next) {
        if (ScamVal_eq(sym, p->key)) {
            gc_unset_root((ScamVal*)val);
            p->val = val;
            return;
        }
    }
    /* The dictionary takes responsibility for the deallocation of the key and value from now on. */
    gc_unset_root((ScamVal*)sym);
    gc_unset_root((ScamVal*)val);
    dct->data[hashval] = ScamDict_list_new(head, sym, val);
    dct->len++;
}


void ScamEnv_insert(ScamEnv* env, ScamStr* key, ScamVal* val) {
    ScamDict_insert((ScamDict*)env, (ScamVal*)key, val);
}


ScamVal* ScamDict_lookup(const ScamDict* dct, const ScamVal* key) {
    size_t hashval = hash(key) % SCAM_DICT_SIZE;
    for (ScamDict_list* p = dct->data[hashval]; p != NULL; p = p->next) {
        if (ScamVal_eq(key, p->key)) {
            return p->val;
        }
    }
    return (ScamVal*)ScamErr_new("key not in dictionary");
}


ScamVal* ScamEnv_lookup(const ScamEnv* env, const ScamStr* key) {
    ScamVal* val = ScamDict_lookup((ScamDict*)env, (const ScamVal*)key);
    if (val->type != SCAM_ERR) {
        return val;
    } else {
        if (ScamEnv_enclosing(env) != NULL) {
            return ScamEnv_lookup(ScamEnv_enclosing(env), key);
        } else {
            return (ScamVal*)ScamErr_new("unbound variable '%s'", ScamStr_unbox((ScamStr*)key));
        }
    }
}


void ScamDict_list_free(ScamDict_list* p) {
    if (p) {
        ScamDict_list_free(p->next);
        free(p);
    }
}


static unsigned long long hash_int(long long x) {
    /* Courtesy of stackoverflow.com/questions/664014/ */
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}


enum { HASH_MULTIPLIER = 31 };
static unsigned long long hash_str(const char* str) {
    /* This function is lightly adapted from section 2.9 of The Practice of Programming, by Brian
     * Kernighan and Rob Pike.
     */
    unsigned long long h = 0;
    for (const unsigned char* p = (const unsigned char*)str; *p != '\0'; p++) {
        h = HASH_MULTIPLIER*h + *p;
    }
    return h;
}

static unsigned long long hash(const ScamVal* v) {
    if (v->type == SCAM_INT) {
        return hash_int(ScamInt_unbox((ScamInt*)v));
    } else if (v->type == SCAM_STR) {
        return hash_str(ScamStr_unbox((ScamStr*)v));
    } else {
        /* Should have a better return value here... */
        return 0;
    }
}

static ScamDict_list* ScamDict_list_new(ScamDict_list* next, ScamVal* key, ScamVal* val) {
    ScamDict_list* ret = gc_malloc(sizeof *ret);
    ret->next = next;
    ret->key = key;
    ret->val = val;
    return ret;
}
