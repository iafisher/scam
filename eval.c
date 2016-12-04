#include "eval.h"
#include "parse.h"

scamval* eval(scamval* v, scamenv* env) {
    if (v->type == SCAM_SYM) {
        return scamenv_lookup(env, v->vals.s);
    } else if (v->type == SCAM_CODE) {
        if (v->vals.arr->count > 0) {
            scamval* fun_val = eval(v->vals.arr->root[0], env);
            if (fun_val->type == SCAM_FUNCTION) {
                scamfun* fun = fun_val->vals.fun;
                // make sure the right number of arguments were given
                if (fun->parameters->count != v->vals.arr->count - 1) {
                    scamval_free(v);
                    scamval_free(fun_val);
                    return scamval_err("wrong number of arguments given");
                }
                scamenv* fun_env = scamenv_init(env);
                for (int i = 0; i < fun->parameters->count; i++) {
                    scamenv_bind(fun_env, fun->parameters->root[i]->vals.s,
                                 eval(v->vals.arr->root[i], env));
                }
                scamval* ret = eval(fun->body, fun_env);
                scamval_free(v);
                scamval_free(fun_val);
                scamenv_free(fun_env);
                return ret;
            } else {
                scamval_free(fun_val);
                return scamval_err("1st element in expression not a function");
            }
        } else {
            scamval_free(v);
            return scamval_err("empty expression");
        }
    } else {
        return v;
    }
}

scamval* eval_line(char* line, scamenv* env) {
    return eval(parse_line(line), env);
}

scamval* eval_file(char* fp, scamenv* env) {
    scamval* exprs = parse_file(fp);
    if (exprs->type == SCAM_CODE) {
        scamval* v = NULL;
        for (int i = 0; i < exprs->vals.arr->count; i++) {
            v = eval(exprs->vals.arr->root[i], env);
            if (i != exprs->vals.arr->count - 1) {
                scamval_free(v);
            }
        }
        return v;
    } else {
        return eval(exprs, env);
    }
}
