#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "scamval.h"



/*** SCAMVAL CONSTRUCTORS ***/
ScamInt* ScamInt_new(long long n) {
    SCAMVAL_NEW(ret, ScamInt, SCAM_INT);
    ret->n = n;
    return ret;
}

ScamDec* ScamDec_new(double d) {
    SCAMVAL_NEW(ret, ScamDec, SCAM_DEC);
    ret->d = d;
    return ret;
}

ScamInt* ScamBool_new(int b) {
    SCAMVAL_NEW(ret, ScamInt, SCAM_BOOL);
    ret->n = b;
    return ret;
}

/* Construct a value that is internally a sequence (lists and S-expressions).
 *   - This only works as long as ScamList and ScamExpr have the same memory footprint.
 */
static ScamSeq* ScamSeq_new(int type) {
    SCAMVAL_NEW(ret, ScamSeq, type);
    ret->count = 0;
    ret->mem_size = 0;
    ret->arr = NULL;
    return ret;
}

/* Construct an internal sequence from a variable argument list. */
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

ScamList* ScamList_new(void) {
    return (ScamList*)ScamSeq_new(SCAM_LIST);
}

ScamList* ScamList_from(size_t n, ...) {
    va_list vlist;
    va_start(vlist, n);
    return (ScamList*)ScamSeq_new_from(SCAM_LIST, n, vlist);
}

ScamExpr* ScamExpr_new(void) {
    return (ScamExpr*)ScamSeq_new(SCAM_SEXPR);
}

ScamExpr* ScamExpr_from(size_t n, ...) {
    va_list vlist;
    va_start(vlist, n);
    return (ScamExpr*)ScamSeq_new_from(SCAM_SEXPR, n, vlist);
}

/* Construct a value that is internally a string (strings, symbols and errors). 
 *   - This will only work as long as ScamStr, ScamSym and ScamErr have the same memory footprint.
 */
static ScamStr* ScamStr_base_new(int type, const char* s) {
    SCAMVAL_NEW(ret, ScamStr, type);
    ret->count = strlen(s);
    ret->mem_size = ret->count + 1;
    ret->s = strdup(s);
    return ret;
}

ScamStr* ScamStr_new(const char* s) {
    return ScamStr_base_new(SCAM_STR, s);
}

ScamStr* ScamStr_from_literal(char* s) {
    size_t n = strlen(s);
    // remove the quotes
    s[n - 1] = '\0';
    memmove(s, s + 1, n);
    n--;
    // fix the backslash escapes
    for (size_t i = 0; i < n; i++) {
        if (s[i] == '\\') {
            switch (s[i + 1]) {
                #define ESCAPE(c, escaped) \
                case escaped: s[i+1] = c; break;
                #define OPTIONAL_ESCAPE(c, escaped) \
                case escaped: s[i+1] = c; break;
                #include "escape.def"
                default: free(s); return (ScamVal*)ScamErr_new("invalid backslash escape");
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

ScamSym* ScamSym_new(const char* s) {
    return (ScamSym*)ScamStr_base_new(SCAM_SYM, s);
}

ScamSym* ScamSym_no_copy(char* s) {
    SCAMVAL_NEW(ret, ScamSym, SCAM_SYM);
    ret->s = s;
    ret->count = ret->mem_size = strlen(s);
    return ret;
}

enum { MAX_ERROR_SIZE = 100 };
ScamErr* ScamErr_new(const char* format, ...) {
    SCAMVAL_NEW(ret, ScamErr, SCAM_ERR);
    va_list vlist;
    va_start(vlist, format);
    ret->s = gc_malloc(MAX_ERROR_SIZE);
    vsnprintf(ret->s, MAX_ERROR_SIZE, format, vlist);
    va_end(vlist);
    return ret;
}

ScamErr* ScamErr_arity(const char* name, size_t got, size_t expected) {
    return ScamErr_new("'%s' got %d arg(s), expected %d", name, got, expected);
}

ScamErr* ScamErr_min_arity(const char* name, size_t got, size_t expected) {
    return ScamErr_new("'%s' got %d arg(s), expected at least %d", name, got, expected);
}

ScamErr* ScamErr_eof(void) {
    return ScamErr_new("reached EOF while reading from a port");
}

ScamFunction* ScamFunction_new(ScamDict* env, ScamList* parameters, ScamExpr* body) {
    SCAMVAL_NEW(ret, ScamFunction, SCAM_LAMBDA);
    ret->env = env;
    ret->parameters = parameters;
    ret->body = body;
    return ret;
}

ScamBuiltin* ScamBuiltin_new(scambuiltin_fun bltin) {
    SCAMVAL_NEW(ret, ScamBuiltin, SCAM_BUILTIN);
    ret->fun = bltin;
    ret->constant = 0;
    return ret;
}

ScamBuiltin* ScamBuiltin_new_const(scambuiltin_fun bltin) {
    SCAMVAL_NEW(ret, ScamBuiltin, SCAM_BUILTIN);
    ret->fun = bltin;
    ret->constant = 1;
    return ret;
}

ScamPort* ScamPort_new(FILE* fp) {
    SCAMVAL_NEW(ret, ScamPort, SCAM_PORT);
    ret->status = (fp == NULL ? SCAMPORT_CLOSED : SCAMPORT_OPEN);
    ret->fp = fp;
    return ret;
}

ScamVal* ScamNull_new(void) {
    SCAMVAL_NEW(ret, ScamVal, SCAM_NULL);
    /* I don't know why this line existed in the first place, but it was causing errors so I
     * commented it out. */
    /*ret->is_root = 0;*/
    return ret;
}


/*** SCAMVAL COMPARISONS ***/
static int ScamVal_numeric_eq(const ScamVal* v1, const ScamVal* v2) {
    if (v1->type == SCAM_INT) {
        if (v2->type == SCAM_INT) {
            return ScamInt_unbox((ScamInt*)v1) == ScamInt_unbox((ScamInt*)v2);
        } else {
            return ScamInt_unbox((ScamInt*)v1) == ScamDec_unbox((ScamDec*)v2);
        }
    } else {
        if (v2->type == SCAM_INT) {
            return ScamDec_unbox((ScamDec*)v1) == ScamInt_unbox((ScamInt*)v2);
        } else {
            return ScamDec_unbox((ScamDec*)v1) == ScamDec_unbox((ScamDec*)v2);
        }
    }
}

static int ScamSeq_eq(const ScamSeq* v1, const ScamSeq* v2) {
    size_t n1 = ScamSeq_len(v1);
    size_t n2 = ScamSeq_len(v2);
    if (n1 == n2) {
        for (int i = 0; i < n1; i++) {
            if (!ScamVal_eq(ScamSeq_get(v1, i), ScamSeq_get(v2, i))) {
                return 0;
            }
        }
        return 1;
    } else {
        return 0;
    }
}

static int ScamDict_eq(const ScamDict* v1, const ScamDict* v2) {
    for (size_t i = 0; i < ScamDict_len(v1); i++) {
        ScamSym* key = ScamDict_key(v1, i);
        ScamVal* val1 = ScamDict_val(v1, i);
        ScamVal* val2 = ScamDict_lookup(v2, key);
        if (!ScamVal_eq(val1, val2)) {
            return 0;
        }
    }
    return 1;
}

int ScamVal_eq(const ScamVal* v1, const ScamVal* v2) {
    if (ScamVal_typecheck(v1, SCAM_NUM) && ScamVal_typecheck(v2, SCAM_NUM)) {
        return ScamVal_numeric_eq(v1, v2);
    } else if (v1->type == v2->type) {
        switch (v1->type) {
            case SCAM_BOOL:
                return ScamBool_unbox((ScamInt*)v1) == ScamBool_unbox((ScamInt*)v2);
            case SCAM_SEXPR:
            case SCAM_LIST:
                return ScamSeq_eq((ScamSeq*)v1, (ScamSeq*)v2);
            case SCAM_SYM:
            case SCAM_STR:
                return (strcmp(ScamStr_unbox((ScamStr*)v1), ScamStr_unbox((ScamStr*)v2)) == 0);
            case SCAM_DICT:
                return ScamDict_eq((ScamDict*)v1, (ScamDict*)v2);
            case SCAM_NULL:
                return 1;
            default:
                return 0;
        }
    } else {
        return 0;
    }
}

static int ScamVal_numeric_gt(const ScamVal* v1, const ScamVal* v2) {
    if (v1->type == SCAM_INT) {
        if (v2->type == SCAM_INT) {
            return ScamInt_unbox((ScamInt*)v1) > ScamInt_unbox((ScamInt*)v2);
        } else {
            return ScamInt_unbox((ScamInt*)v1) > ScamDec_unbox((ScamDec*)v2);
        }
    } else {
        if (v2->type == SCAM_INT) {
            return ScamDec_unbox((ScamDec*)v1) > ScamInt_unbox((ScamInt*)v2);
        } else {
            return ScamDec_unbox((ScamDec*)v1) > ScamDec_unbox((ScamDec*)v2);
        }
    }
}

int ScamVal_gt(const ScamVal* v1, const ScamVal* v2) {
    if (ScamVal_typecheck(v1, SCAM_NUM) && ScamVal_typecheck(v2, SCAM_NUM)) {
        return ScamVal_numeric_gt(v1, v2);
    } else if (ScamVal_typecheck(v1, SCAM_STR) && 
               ScamVal_typecheck(v2, SCAM_STR)) {
        return strcmp(((ScamStr*)v1)->s, ((ScamStr*)v2)->s) > 0;
    } else {
        return 0;
    }
}


/*** NUMERIC API ***/
long long ScamInt_unbox(const ScamInt* v) { return v->n; }
long long ScamBool_unbox(const ScamInt* v) { return v->n; }
double ScamDec_unbox(const ScamDec* v) {
    if (v->type == SCAM_DEC) {
        return ((ScamDec*)v)->d; 
    } else {
        return ((ScamInt*)v)->n;
    }
}


/*** SEQUENCE API ***/
// constants for the ScamSeq_grow function
enum { SEQ_SIZE_INITIAL = 5, SEQ_SIZE_GROW = 2};

// Grow the size of the sequence in memory so that it is at least the given minimum size, and 
// possibly larger
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

// Unlike ScamSeq_grow, the new sequence is guaranteed to be exactly the new size provided
static void ScamSeq_resize(ScamSeq* seq, size_t new_sz) {
    seq->mem_size = new_sz;
    if (seq->arr == NULL) {
        seq->arr = gc_malloc(seq->mem_size * sizeof *seq->arr);
    } else {
        seq->arr = gc_realloc(seq->arr, seq->mem_size * sizeof *seq->arr);
    }
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


/*** FUNCTION API ***/
size_t ScamFunction_nparams(const ScamFunction* f) {
    return ScamSeq_len((ScamSeq*)f->parameters);
}

ScamSym* ScamFunction_param(const ScamFunction* f, size_t i) {
    return (ScamSym*)gc_copy_ScamVal(ScamSeq_get((ScamSeq*)f->parameters, i));
}

ScamExpr* ScamFunction_body(const ScamFunction* f) {
    return (ScamExpr*)gc_copy_ScamVal((ScamVal*)f->body);
}

ScamDict* ScamFunction_env(const ScamFunction* f) {
    return ScamDict_new(f->env);
}

const ScamDict* ScamFunction_env_ref(const ScamFunction* f) {
    return f->env;
}

scambuiltin_fun ScamBuiltin_function(const ScamBuiltin* f) {
    return f->fun;
}

int ScamBuiltin_is_const(const ScamBuiltin* f) {
    return f->constant;
}


/*** STRING API ***/
static void ScamStr_resize(ScamStr* sbox, size_t new_sz) {
    sbox->mem_size = new_sz;
    if (sbox->s == NULL) {
        sbox->s = gc_malloc(new_sz);
    } else {
        sbox->s = gc_realloc(sbox->s, new_sz);
    }
}

const char* ScamStr_unbox(const ScamStr* sbox) { return sbox->s; }

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


/*** PORT API ***/
FILE* ScamPort_unbox(ScamPort* v) { return v->fp; }
int ScamPort_status(const ScamPort* v) { return v->status; }

void ScamPort_set_status(ScamPort* v, int new_status) {
    v->status = new_status;
}


/*** DICTIONARY API ***/
ScamDict* ScamDict_new(ScamDict* enclosing) {
    SCAMVAL_NEW(ret, ScamDict, SCAM_ANY);
    ret->enclosing = enclosing;
    /* The order is very important here: if ret was constructed as a SCAM_DICT right away, then the 
     * garbage collector might try to access syms or vals before they were allocated. The two calls 
     * to ScamList_new are safe because if the first call invokes the collector, the second call
     * cannot as the collector will allocate space for at least one additional object.
     */
    ret->syms = ScamList_new();
    ret->vals = ScamList_new();
    ret->type = SCAM_DICT;
    gc_unset_root((ScamVal*)ret->syms);
    gc_unset_root((ScamVal*)ret->vals);
    return ret;
}

ScamDict* ScamDict_from(size_t n, ...) {
    va_list vlist;
    va_start(vlist, n);
    ScamDict* ret = ScamDict_new(NULL);
    for (int i = 0; i < n; i++) {
        ScamVal* key_val_pair = va_arg(vlist, ScamVal*);
        if (key_val_pair->type == SCAM_LIST && ScamSeq_len((ScamSeq*)key_val_pair) == 2) {
            ScamSym* key = (ScamSym*)ScamSeq_get((ScamSeq*)key_val_pair, 0);
            ScamVal* val = ScamSeq_get((ScamSeq*)key_val_pair, 1);
            ScamDict_bind(ret, key, val);
        } else {
            gc_unset_root((ScamVal*)ret);
            return (ScamDict*)ScamErr_new("ScamDict_from takes key-value pairs as arguments");
        }
    }
    va_end(vlist);
    return ret;
}

ScamList* ScamDict_keys(const ScamDict* dct) { return dct->syms; }
ScamList* ScamDict_vals(const ScamDict* dct) { return dct->vals; }
ScamDict* ScamDict_enclosing(const ScamDict* dct) { 
    return dct->enclosing; 
}

void ScamDict_set_keys(ScamDict* dct, ScamList* new_keys) {
    gc_unset_root((ScamVal*)ScamDict_keys(dct));
    dct->syms = new_keys;
    gc_unset_root((ScamVal*)new_keys);
}

void ScamDict_set_vals(ScamDict* dct, ScamList* new_vals) {
    gc_unset_root((ScamVal*)ScamDict_vals(dct));
    dct->vals = new_vals;
    gc_unset_root((ScamVal*)new_vals);
}

size_t ScamDict_len(const ScamDict* dct) {
    return ScamSeq_len((ScamSeq*)dct->syms);
}

ScamSym* ScamDict_key(const ScamDict* dct, size_t i) {
    return (ScamSym*)ScamSeq_get((ScamSeq*)ScamDict_keys(dct), i);
}

ScamVal* ScamDict_val(const ScamDict* dct, size_t i) {
    return ScamSeq_get((ScamSeq*)ScamDict_vals(dct), i);
}

void ScamDict_bind(ScamDict* dct, ScamSym* sym, ScamVal* val) {
    gc_unset_root((ScamVal*)sym);
    gc_unset_root((ScamVal*)val);
    if (sym->type == SCAM_PORT || sym->type == SCAM_LAMBDA || sym->type == SCAM_BUILTIN || 
        sym->type == SCAM_NULL) {
        // unbindable types
        return;
        //return scamerr("cannot bind type '%s'", scamtype_name(sym->type));
    }
    for (int i = 0; i < ScamDict_len(dct); i++) {
        if (ScamVal_eq((ScamVal*)ScamDict_key(dct, i), (ScamVal*)sym)) {
            gc_unset_root((ScamVal*)ScamDict_val(dct, i));
            ScamSeq_set((ScamSeq*)ScamDict_vals(dct), i, val);
            return;
        }
    }
    ScamSeq_append((ScamSeq*)ScamDict_keys(dct), (ScamVal*)sym);
    ScamSeq_append((ScamSeq*)ScamDict_vals(dct), val);
}

ScamVal* ScamDict_lookup(const ScamDict* dct, const ScamSym* key) {
    for (int i = 0; i < ScamDict_len(dct); i++) {
        ScamSym* this_key = ScamDict_key(dct, i);
        if (ScamVal_eq((ScamVal*)key, (ScamVal*)this_key)) {
            return ScamDict_val(dct, i);
        }
    }
    if (ScamDict_enclosing(dct) != NULL) {
        return ScamDict_lookup(ScamDict_enclosing(dct), key);
    } else {
        if (key->type == SCAM_STR) {
            return (ScamVal*)ScamErr_new("unbound variable '%s'", ScamStr_unbox((ScamStr*)key));
        } else {
            return (ScamVal*)ScamErr_new("unbound variable");
        }
    }
}

void ScamStr_write(const ScamStr* v, FILE* fp);
void ScamSeq_write(const ScamSeq* v, const char* start, const char* mid, const char* end, FILE* fp);
void ScamDict_write(const ScamDict* v, FILE* fp);

void ScamVal_write(const ScamVal* v, FILE* fp) {
    if (!v) return;
    switch (v->type) {
        case SCAM_INT: fprintf(fp, "%lli", ScamInt_unbox((ScamInt*)v)); break;
        case SCAM_DEC: fprintf(fp, "%f", ScamDec_unbox((ScamDec*)v)); break;
        case SCAM_BOOL: fprintf(fp, "%s", ScamBool_unbox((ScamInt*)v) ? "true" : "false"); break;
        case SCAM_LIST: ScamSeq_write((ScamSeq*)v, "[", " ", "]", fp); break;
        case SCAM_SEXPR: ScamSeq_write((ScamSeq*)v, "(", " ", ")", fp); break;
        case SCAM_LAMBDA: fprintf(fp, "<Scam function>"); break;
        case SCAM_BUILTIN: fprintf(fp, "<Scam builtin>"); break;
        case SCAM_PORT: fprintf(fp, "<Scam port>"); break;
        case SCAM_STR: ScamStr_write((ScamStr*)v, fp); break;
        case SCAM_SYM: fprintf(fp, "%s", ScamStr_unbox((ScamStr*)v)); break;
        case SCAM_ERR: fprintf(fp, "Error: %s", ScamStr_unbox((ScamStr*)v)); break;
        case SCAM_DICT: ScamDict_write((ScamDict*)v, fp); break;
        //default: fprintf(fp, "null");
    }
}

void ScamStr_write(const ScamStr* sbox, FILE* fp) {
    fputc('"', fp);
    for (size_t i = 0; i < ScamStr_len(sbox); i++) {
        char c = ScamStr_get(sbox, i);
        switch (c) {
            #define ESCAPE(c, escaped) \
            case c: fputc('\\', fp); fputc(escaped, fp); break;
            #define OPTIONAL_ESCAPE(c, escaped) \
            case c: fputc(escaped, fp); break;
            #include "escape.def"
            default: fputc(c, fp);
        }
    }
    fputc('"', fp);
}

void ScamSeq_write(const ScamSeq* seq, const char* start, const char* mid, const char* end,
                   FILE* fp) {
    fprintf(fp, "%s", start);
    for (size_t i = 0; i < ScamSeq_len(seq); i++) {
        ScamVal_write(ScamSeq_get(seq, i), fp);
        if (i != ScamSeq_len(seq) - 1) {
            fprintf(fp, "%s", mid);
        }
    }
    fprintf(fp, "%s", end);
}

void ScamDict_write(const ScamDict* dct, FILE* fp) {
    fputc('{', fp);
    for (size_t i = 0; i < ScamDict_len(dct); i++) {
        ScamVal_write((ScamVal*)ScamDict_key(dct, i), fp);
        fputc(':', fp);
        ScamVal_write(ScamDict_val(dct, i), fp);
        if (i != ScamDict_len(dct) - 1) {
            fputc(' ', fp);
        }
    }
    fputc('}', fp);
}

char* ScamVal_to_str(const ScamVal* v) {
    if (!v) return NULL;
    if (v->type == SCAM_STR) {
        return strdup(ScamStr_unbox((ScamStr*)v));
    } else {
        return ScamVal_to_repr(v);
    }
}

char* ScamVal_to_repr(const ScamVal* v) {
    if (!v) return NULL;
    char* ret;
    size_t n;
    FILE* stream = open_memstream(&ret, &n);
    ScamVal_write(v, stream);
    fclose(stream);
    return ret;
}

void ScamVal_print(const ScamVal* v) {
    if (!v || v->type == SCAM_NULL) return;
    ScamVal_write(v, stdout);
}

void ScamVal_println(const ScamVal* v) {
    if (!v || v->type == SCAM_NULL) return;
    ScamVal_print(v);
    printf("\n");
}

void ScamVal_print_debug(const ScamVal* v) {
    ScamVal_print(v);
    printf(" (%s)", scamtype_debug_name(v->type));
}

void ScamVal_print_ast(const ScamVal* ast, int indent) {
    for (int i = 0; i < indent; i++)
        printf("  ");
    if (ast->type == SCAM_SEXPR) {
        size_t n = ScamSeq_len((ScamSeq*)ast);
        if (n == 0) {
            printf("EMPTY EXPR%s\n", ast->is_root ? " (root)" : "");
        } else {
            printf("EXPR%s\n", ast->is_root ? " (root)" : "");
            for (int i = 0; i < ScamSeq_len((ScamSeq*)ast); i++) {
                ScamVal_print_ast(ScamSeq_get((ScamSeq*)ast, i), indent + 1);
            }
        }
    } else {
        ScamVal_print(ast);
        printf("%s\n", ast->is_root ? " (root)" : "");
    }
}


/*** TYPECHECKING ***/
int ScamVal_typecheck(const ScamVal* v, int type) {
    switch (type) {
        case SCAM_ANY: 
            return 1;
        case SCAM_SEQ: 
            return v->type == SCAM_LIST || v->type == SCAM_STR;
        case SCAM_CONTAINER:
            return v->type == SCAM_LIST || v->type == SCAM_STR || v->type == SCAM_DICT;
        case SCAM_NUM: 
            return v->type == SCAM_INT || v->type == SCAM_DEC;
        case SCAM_CMP: 
            return v->type == SCAM_STR || v->type == SCAM_INT || v->type == SCAM_DEC;
        case SCAM_FUNCTION: 
            return v->type == SCAM_LAMBDA || v->type == SCAM_BUILTIN;
        default: 
            return v->type == type;
    }
}

int is_numeric_type(int type) { return type == SCAM_INT || type == SCAM_DEC; }
int is_seq_type(int type) { return type == SCAM_LIST || type == SCAM_STR; }
int is_container_type(int type) {
    return type == SCAM_LIST || type == SCAM_STR || type == SCAM_DICT;
}


int narrowest_type(int type1, int type2) {
    if (type1 == type2) {
        return type1;
    } else if (is_numeric_type(type1) && is_numeric_type(type2)) {
        return SCAM_NUM;
    } else if (is_seq_type(type1) && is_seq_type(type2)) {
        return SCAM_SEQ;
    } else if (is_container_type(type1) && is_container_type(type2)) {
        return SCAM_CONTAINER;
    } else {
        return SCAM_ANY;
    }
}

int ScamSeq_narrowest_type(ScamSeq* args) {
    size_t n = ScamSeq_len(args);
    if (n == 0) return SCAM_ANY;
    int type_so_far = ScamSeq_get(args, 0)->type;
    for (int i = 1; i < n; i++) {
        type_so_far = narrowest_type(ScamSeq_get(args, i)->type, type_so_far);
    }
    return type_so_far;
}

ScamErr* ScamErr_type(const char* name, size_t pos, int got, int expected) {
    return ScamErr_new("'%s' got %s as arg %d, expected %s", name, scamtype_name(got), pos+1, 
                       scamtype_name(expected));
}

const char* scamtype_name(int type) {
    switch (type) {
        #define EXPAND_TYPE(type_val, type_name) \
            case type_val: return type_name;
        #include "type.def"
        /*
        case SCAM_INT: return "integer";
        case SCAM_DEC: return "decimal";
        case SCAM_BOOL: return "boolean";
        case SCAM_LIST: return "list";
        case SCAM_STR: return "string";
        case SCAM_LAMBDA: return "function";
        case SCAM_PORT: return "port";
        case SCAM_BUILTIN: return "builtin";
        case SCAM_SEXPR: return "S-expression";
        case SCAM_SYM: return "symbol";
        case SCAM_ERR: return "error";
        case SCAM_NULL: return "null";
        case SCAM_DICT: return "dictionary";
        // abstract types
        case SCAM_SEQ: return "list or string";
        case SCAM_CONTAINER: return "list, string or dictionary";
        case SCAM_NUM: return "integer or decimal";
        case SCAM_CMP: return "integer, decimal or string";
        case SCAM_ANY: return "any value";
        */
        default: return "bad ScamVal type";
    }
}

const char* scamtype_debug_name(int type) {
    switch (type) {
        #define EXPAND_TYPE(type_val, type_name) \
            case type_val: return #type_val ;
        #include "type.def"
        default: return "bad ScamVal type";
    }
}
