#include "collector.h"
#include "scamval.h"


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


ScamBool* ScamBool_new(bool b) {
    SCAMVAL_NEW(ret, ScamBool, SCAM_BOOL);
    ret->b = b;
    return ret;
}


long long ScamInt_unbox(const ScamInt* v) {
    return v->n;
}


bool ScamBool_unbox(const ScamBool* v) {
    return v->b;
}


double ScamDec_unbox(const ScamDec* v) {
    if (v->type == SCAM_DEC) {
        return ((ScamDec*)v)->d;
    } else {
        return ((ScamInt*)v)->n;
    }
}
