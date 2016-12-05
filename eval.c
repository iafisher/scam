#include <string.h>
#include "eval.h"
#include "parse.h"

// Forward declaration of various eval utilities
scamval* eval_define(scamval*, scamenv*);
scamval* eval_if(scamval*, scamenv*);
scamval* eval_apply(scamval*, scamenv*);

scamval* eval(scamval* ast, scamenv* env) {
    if (ast->type == SCAM_SYM) {
        return scamenv_lookup(env, ast);
    } else if (ast->type == SCAM_CODE) {
        if (scamval_len(ast) == 0) {
            return scamval_err("empty expression");
        }
        // define statement and if expression are special cases
        if (strcmp(scamval_get(ast, 0)->vals.s, "define") == 0) {
            return eval_define(ast, env);
        } else if (strcmp(scamval_get(ast, 0)->vals.s, "if") == 0) {
            return eval_if(ast, env);
        } else {
            return eval_apply(ast, env);
        }
    } else if (ast->type == SCAM_LIST) {
        scamval* ret = scamval_list();
        for (int i = 0; i < scamval_len(ast); i++) {
            scamval_append(ret, eval(scamval_get(ast, i), env));
        }
        return ret;
    } else {
        return scamval_copy(ast);
    }
}

scamval* eval_line(char* line, scamenv* env) {
    scamval* ast = parse_line(line);
    scamval* ret = eval(ast, env);
    scamval_free(ast);
    return ret;
}

scamval* eval_file(char* fp, scamenv* env) {
    scamval* exprs = parse_file(fp);
    if (exprs->type == SCAM_CODE) {
        scamval* v = NULL;
        for (int i = 0; i < scamval_len(exprs); i++) {
            v = eval(scamval_get(exprs, i), env);
            if (i != scamval_len(exprs) - 1) { scamval_free(v);
            }
        }
        scamval_free(exprs);
        return v;
    } else {
        scamval* ret = eval(exprs, env);
        scamval_free(exprs);
        return ret;
    }
}

// Evaluate a define statement
scamval* eval_define(scamval* ast, scamenv* env) {
    if (scamval_len(ast) != 3) {
        return scamval_err("wrong number of arguments to 'define'");
    } else if (scamval_get(ast, 1)->type != SCAM_SYM) {
        return scamval_err("cannot define non-symbol");
    } else {
        scamval* k = scamval_copy(scamval_get(ast, 1));
        scamval* v = eval(scamval_get(ast, 2), env);
        scamenv_bind(env, k, v);
        return scamval_null();
    }
}

// Evaluate an if statement
scamval* eval_if(scamval* ast, scamenv* env) {
    if (scamval_len(ast) != 4) {
        return scamval_err("wrong number of arguments to 'if'");
    } else {
        scamval* cond = eval(scamval_get(ast, 1), env);
        if (cond->type == SCAM_BOOL) {
            if (cond->vals.n) {
                scamval_free(cond);
                return eval(scamval_get(ast, 2), env);
            } else {
                scamval_free(cond);
                return eval(scamval_get(ast, 3), env);
            }
        } else {
            scamval_free(cond);
            return scamval_err("condition of an if expression must be a bool");
        }
    }
}

// Evaluate a function application
scamval* eval_apply(scamval* ast, scamenv* env) {
    // evaluate the function and the argument list
    scamval* arglist = scamval_code();
    for (int i = 0; i < scamval_len(ast); i++) {
        scamval* v = eval(scamval_get(ast, i), env);
        if (v->type != SCAM_ERR) {
            scamval_append(arglist, v);
        } else {
            scamval_free(arglist);
            return v;
        }
    }
    scamval* fun_val = scamval_pop(arglist, 0);
    if (fun_val->type == SCAM_FUNCTION) {
        // extract the function from the scamval, for convenience
        scamfun* fun = fun_val->vals.fun;
        // make sure the right number of arguments were given
        if (fun->parameters->count != scamval_len(arglist)) {
            scamval_free(arglist);
            return scamval_err("wrong number of arguments given");
        }
        scamenv* fun_env = scamenv_init(env);
        for (int i = 1; i < fun->parameters->count; i++) {
            scamenv_bind(fun_env, fun->parameters->root[i - 1],
                                  scamval_get(arglist, i));
        }
        scamval* ret = eval(fun->body, fun_env);
        scamenv_free(fun_env); scamval_free(arglist); scamval_free(fun_val);
        return ret;
    } else if (fun_val->type == SCAM_BUILTIN) {
        scamval* ret = fun_val->vals.bltin(arglist);
        scamval_free(fun_val);
        return ret;
    } else {
        scamval_free(arglist); scamval_free(fun_val);
        return scamval_err("first element in expression not a function");
    }
}
