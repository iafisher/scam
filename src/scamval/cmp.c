#include <string.h>
#include "scamval.h"


static int ScamVal_numeric_eq(const ScamVal*, const ScamVal*);
static int ScamSeq_eq(const ScamSeq*, const ScamSeq*);
static int ScamDict_eq(const ScamDict*, const ScamDict*);
static int ScamVal_numeric_gt(const ScamVal*, const ScamVal*);


int ScamVal_eq(const ScamVal* v1, const ScamVal* v2) {
    if (ScamVal_typecheck(v1, SCAM_NUM) && ScamVal_typecheck(v2, SCAM_NUM)) {
        return ScamVal_numeric_eq(v1, v2);
    } else if (v1->type == v2->type) {
        switch (v1->type) {
            case SCAM_BOOL:
                return ScamBool_unbox((ScamBool*)v1) == ScamBool_unbox((ScamBool*)v2);
            case SCAM_SEXPR:
            case SCAM_LIST:
                return ScamSeq_eq((ScamSeq*)v1, (ScamSeq*)v2);
            case SCAM_SYM:
            case SCAM_STR:
                return (strcmp(ScamStr_unbox((ScamStr*)v1), ScamStr_unbox((ScamStr*)v2)) == 0);
            case SCAM_ENV:
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


int ScamVal_gt(const ScamVal* v1, const ScamVal* v2) {
    if (ScamVal_typecheck(v1, SCAM_NUM) && ScamVal_typecheck(v2, SCAM_NUM)) {
        return ScamVal_numeric_gt(v1, v2);
    } else if (ScamVal_typecheck(v1, SCAM_STR) && ScamVal_typecheck(v2, SCAM_STR)) {
        return strcmp(((ScamStr*)v1)->s, ((ScamStr*)v2)->s) > 0;
    } else {
        return 0;
    }
}


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
        for (size_t i = 0; i < n1; i++) {
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
    for (size_t i = 0; i < SCAM_DICT_SIZE; i++) {
        for (ScamDict_list* p = v1->data[i]; p != NULL; p = p->next) {
            ScamVal* val2 = ScamDict_lookup(v2, p->key);
            if (!ScamVal_eq(p->val, val2)) {
                return 0;
            }
        }
    }
    return 1;
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
