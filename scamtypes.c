#include "scamtypes.h"

int scamval_typecheck(scamval* v, int type) {
    switch (type) {
        case SCAM_ANY: return 1;
        case SCAM_SEQ: return v->type == SCAM_LIST || v->type == SCAM_STR;
        case SCAM_NUM: return v->type == SCAM_INT || v->type == SCAM_DEC;
        default: return v->type == type;
    }
}

int narrowest_type(int type1, int type2) {
    if (type1 == type2) {
        return type1;
    } else if ((type1 == SCAM_DEC && type2 == SCAM_INT) ||
               (type1 == SCAM_INT && type2 == SCAM_DEC)) {
        return SCAM_NUM;
    } else if ((type1 == SCAM_STR && type2 == SCAM_LIST) ||
               (type1 == SCAM_LIST && type2 == SCAM_STR)) {
        return SCAM_SEQ;
    } else {
        return SCAM_ANY;
    }
}

int scamseq_narrowest_type(scamval* args) {
    size_t n = scamseq_len(args);
    if (n == 0) return SCAM_ANY;
    int type_so_far = scamseq_get(args, 0)->type;
    for (int i = 1; i < n; i++) {
        type_so_far = narrowest_type(scamseq_get(args, i)->type, type_so_far);
    }
    return type_so_far;
}

scamval* scamerr_type(const char* name, size_t pos, int got, int expected) {
    return scamerr("'%s' got %s as arg %d, expected %s", name, 
                   scamtype_name(got), pos + 1, scamtype_name(expected));
}

const char* scamtype_name(int type) {
    switch (type) {
        case SCAM_INT: return "integer";
        case SCAM_DEC: return "decimal";
        case SCAM_BOOL: return "boolean";
        case SCAM_LIST: return "list";
        case SCAM_STR: return "string";
        case SCAM_FUNCTION: return "function";
        case SCAM_PORT: return "port";
        case SCAM_BUILTIN: return "function";
        case SCAM_SEXPR: return "S-expression";
        case SCAM_SYM: return "symbol";
        case SCAM_ERR: return "error";
        case SCAM_NULL: return "null";
        case SCAM_SEQ: return "list or string";
        case SCAM_NUM: return "integer or decimal";
        case SCAM_ANY: return "any value";
        default: return "bad scamval type";
    }
}

const char* scamtype_debug_name(int type) {
    switch (type) {
        case SCAM_INT: return "SCAM_INT";
        case SCAM_DEC: return "SCAM_DEC";
        case SCAM_BOOL: return "SCAM_BOOL";
        case SCAM_LIST: return "SCAM_LIST";
        case SCAM_STR: return "SCAM_STR";
        case SCAM_FUNCTION: return "SCAM_FUNCTION";
        case SCAM_PORT: return "SCAM_PORT";
        case SCAM_BUILTIN: return "SCAM_BUILTIN";
        case SCAM_SEXPR: return "SCAM_SEXPR";
        case SCAM_SYM: return "SCAM_SYM";
        case SCAM_ERR: return "SCAM_ERR";
        case SCAM_NULL: return "SCAM_NULL";
        case SCAM_SEQ: return "SCAM_SEQ";
        case SCAM_NUM: return "SCAM_NUM";
        case SCAM_ANY: return "SCAM_ANY";
        default: return "bad scamval type";
    }
}
