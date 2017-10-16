#include <stdio.h>
#include <string.h>
#include "collector.h"
#include "scamval.h"


static void ScamStr_write(const ScamStr* v, FILE* fp);
static void ScamSeq_write(const ScamSeq* v, const char* start, const char* mid, const char* end, 
                          FILE* fp);
static void ScamDict_write(const ScamDict* v, FILE* fp);


ScamFunction* ScamFunction_new(ScamDict* env, ScamSeq* parameters, ScamSeq* body) {
    SCAMVAL_NEW(ret, ScamFunction, SCAM_FUNCTION);
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
    return ret;
}


size_t ScamFunction_nparams(const ScamFunction* f) {
    return ScamSeq_len(f->parameters);
}


ScamStr* ScamFunction_param(const ScamFunction* f, size_t i) {
    return (ScamStr*)gc_copy_ScamVal(ScamSeq_get(f->parameters, i));
}


ScamSeq* ScamFunction_body(const ScamFunction* f) {
    return (ScamSeq*)gc_copy_ScamVal((ScamVal*)f->body);
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


FILE* ScamPort_unbox(ScamPort* v) { 
    return v->fp; 
}


int ScamPort_status(const ScamPort* v) { 
    return v->status; 
}


void ScamPort_set_status(ScamPort* v, int new_status) {
    v->status = new_status;
}


void ScamVal_write(const ScamVal* v, FILE* fp) {
    if (!v) return;
    switch (v->type) {
        case SCAM_INT: fprintf(fp, "%lli", ScamInt_unbox((ScamInt*)v)); break;
        case SCAM_DEC: fprintf(fp, "%f", ScamDec_unbox((ScamDec*)v)); break;
        case SCAM_BOOL: fprintf(fp, "%s", ScamBool_unbox((ScamInt*)v) ? "true" : "false"); break;
        case SCAM_LIST: ScamSeq_write((ScamSeq*)v, "[", " ", "]", fp); break;
        case SCAM_SEXPR: ScamSeq_write((ScamSeq*)v, "(", " ", ")", fp); break;
        case SCAM_FUNCTION: fprintf(fp, "<Scam function>"); break;
        case SCAM_BUILTIN: fprintf(fp, "<Scam builtin>"); break;
        case SCAM_PORT: fprintf(fp, "<Scam port>"); break;
        case SCAM_STR: ScamStr_write((ScamStr*)v, fp); break;
        case SCAM_SYM: fprintf(fp, "%s", ScamStr_unbox((ScamStr*)v)); break;
        case SCAM_ERR: fprintf(fp, "Error: %s", ScamStr_unbox((ScamStr*)v)); break;
        case SCAM_DICT: ScamDict_write((ScamDict*)v, fp); break;
        default: break;
    }
}


static void ScamStr_write(const ScamStr* sbox, FILE* fp) {
    fputc('"', fp);
    for (size_t i = 0; i < ScamStr_len(sbox); i++) {
        char c = ScamStr_get(sbox, i);
        switch (c) {
            #define ESCAPE(c, escaped) \
            case c: fputc('\\', fp); fputc(escaped, fp); break;
            #define OPTIONAL_ESCAPE(c, escaped) \
            case c: fputc(escaped, fp); break;
            #include "../escape.def"
            default: fputc(c, fp);
        }
    }
    fputc('"', fp);
}


static void ScamSeq_write(const ScamSeq* seq, const char* start, const char* mid, const char* end,
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


static void ScamDict_write(const ScamDict* dct, FILE* fp) {
    size_t remaining = ScamDict_len(dct);
    fputc('{', fp);
    for (size_t i = 0; i < SCAM_DICT_SIZE && remaining > 0; i++) {
        for (ScamDict_list* p = dct->data[i]; p != NULL; p = p->next) {
            ScamVal_write(p->key, fp);
            fputc(':', fp);
            ScamVal_write(p->val, fp);
            remaining--;
            if (remaining > 0) {
                fputc(' ', fp);
            }
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


int ScamVal_typecheck(const ScamVal* v, enum ScamType type) {
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
        case SCAM_BASE_FUNCTION: 
            return v->type == SCAM_FUNCTION || v->type == SCAM_BUILTIN;
        default: 
            return v->type == type;
    }
}


int is_numeric_type(enum ScamType type) { 
    return type == SCAM_INT || type == SCAM_DEC; 
}


int is_seq_type(enum ScamType type) { 
    return type == SCAM_LIST || type == SCAM_STR; 
}


int is_container_type(enum ScamType type) {
    return type == SCAM_LIST || type == SCAM_STR || type == SCAM_DICT;
}


enum ScamType narrowest_type(enum ScamType type1, enum ScamType type2) {
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


enum ScamType ScamSeq_narrowest_type(ScamSeq* args) {
    size_t n = ScamSeq_len(args);
    if (n == 0) return SCAM_ANY;
    int type_so_far = ScamSeq_get(args, 0)->type;
    for (int i = 1; i < n; i++) {
        type_so_far = narrowest_type(ScamSeq_get(args, i)->type, type_so_far);
    }
    return type_so_far;
}


ScamStr* ScamErr_type(const char* name, size_t pos, enum ScamType got, enum ScamType expected) {
    return ScamErr_new("'%s' got %s as arg %d, expected %s", name, scamtype_name(got), pos+1, 
                       scamtype_name(expected));
}


const char* scamtype_name(enum ScamType type) {
    switch (type) {
        #define EXPAND_TYPE(type_val, type_name) \
            case type_val: return type_name;
        #include "../type.def"
        default: return "bad ScamVal type";
    }
}


const char* scamtype_debug_name(enum ScamType type) {
    switch (type) {
        #define EXPAND_TYPE(type_val, type_name) \
            case type_val: return #type_val ;
        #include "../type.def"
        default: return "bad ScamVal type";
    }
}
