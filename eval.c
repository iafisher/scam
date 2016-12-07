#include <string.h>
#include "eval.h"
#include "parse.h"

// Forward declaration of various eval utilities
scamval* eval_define(scamval*, scamenv*);
scamval* eval_lambda(scamval*, scamenv*);
scamval* eval_if(scamval*, scamenv*);
scamval* eval_and(scamval*, scamenv*);
scamval* eval_or(scamval*, scamenv*);
scamval* eval_eval(scamval*, scamenv*);
scamval* eval_apply(scamval*, scamenv*);

scamval* eval(scamval* ast, scamenv* env) {
    if (ast->type == SCAM_SYM) {
        scamval* ret = scamenv_lookup(env, ast);
        scamval_free(ast);
        return ret;
    } else if (ast->type == SCAM_CODE) {
        if (scamval_len(ast) == 0) {
            scamval_free(ast);
            return scamval_err("empty expression");
        }
        // handle special expressions and statements
        if (scamval_get(ast, 0)->type == SCAM_SYM) {
            char* name = scamval_get(ast, 0)->vals.s;
            if (strcmp(name, "define") == 0) {
                return eval_define(ast, env);
            } else if (strcmp(name, "if") == 0) {
                return eval_if(ast, env);
            } else if (strcmp(name, "lambda") == 0) {
                return eval_lambda(ast, env);
            } else if (strcmp(name, "and") == 0) {
                return eval_and(ast, env);
            } else if (strcmp(name, "or") == 0) {
                return eval_or(ast, env);
            } else if (strcmp(name, "eval") == 0) {
                return eval_eval(ast, env);
            }
        }
        return eval_apply(ast, env);
    } else if (ast->type == SCAM_LIST) {
        for (int i = 0; i < scamval_len(ast); i++) {
            scamval_set(ast, i, eval(scamval_get(ast, i), env));
        }
        return ast;
    } else {
        return ast;
    }
}

scamval* eval_line(char* line, scamenv* env) {
    return eval(parse_line(line), env);
}

scamval* eval_file(char* fp, scamenv* env) {
    scamval* exprs = parse_file(fp);
    if (exprs->type == SCAM_CODE) {
        scamval* v = NULL;
        for (int i = 0; i < scamval_len(exprs); i++) {
            v = eval(scamval_get(exprs, i), env);
            if (i != scamval_len(exprs) - 1) { 
                scamval_free(v);
            }
        }
        scamval_free(exprs);
        return v;
    } else {
        return eval(exprs, env);
    }
}

// Evaluate a lambda expression
scamval* eval_lambda(scamval* ast, scamenv* env) {
    size_t n = scamval_len(ast);
    if (n != 3) {
        scamval_free(ast);
        return scamval_err("'lambda' expected %d argument(s), got %d", 3, n);
    } else {
        scamval* parameters = scamval_pop(ast, 1);
        scamval* body = scamval_pop(ast, 1);
        scamval_free(ast);
        if (parameters->type != SCAM_CODE) {
            scamval_free(parameters); 
            scamval_free(body);
            return scamval_err("arg 2 to 'lambda' should be expression");
        }
        for (int i = 0; i < scamval_len(parameters); i++) {
            if (scamval_get(body, i)->type != SCAM_SYM) {
                scamval_free(parameters); 
                scamval_free(body);
                return scamval_err("lambda parameter must be symbol");
            }
        }
        return scamval_function(env, parameters, body);
    }
}

// Evaluate a define statement
scamval* eval_define(scamval* ast, scamenv* env) {
    if (scamval_len(ast) != 3) {
        scamval_free(ast);
        return scamval_err("wrong number of arguments to 'define'");
    } else if (scamval_get(ast, 1)->type != SCAM_SYM) {
        scamval_free(ast);
        return scamval_err("cannot define non-symbol");
    } else {
        scamval* k = scamval_pop(ast, 1);
        scamval* v = eval(scamval_pop(ast, 1), env);
        scamval_free(ast);
        scamenv_bind(env, k, v);
        return scamval_null();
    }
}

// Evaluate an if expression
scamval* eval_if(scamval* ast, scamenv* env) {
    if (scamval_len(ast) != 4) {
        scamval_free(ast);
        return scamval_err("'if' passed wrong number of arguments");
    } else {
        scamval* cond = eval(scamval_pop(ast, 1), env);
        if (cond->type == SCAM_BOOL) {
            scamval* true_clause = scamval_pop(ast, 1);
            scamval* false_clause = scamval_pop(ast, 1);
            scamval_free(ast);
            if (cond->vals.n) {
                scamval_free(cond);
                scamval_free(false_clause);
                return eval(true_clause, env);
            } else {
                scamval_free(cond);
                scamval_free(true_clause);
                return eval(false_clause, env);
            }
        } else {
            scamval_free(cond);
            scamval_free(ast);
            return scamval_err("condition of an if expression must be a bool");
        }
    }
}

// Evaluate an and expression
scamval* eval_and(scamval* ast, scamenv* env) {
    while (scamval_len(ast) > 0) {
        scamval* v = eval(scamval_pop(ast, 0), env);
        if (v->type != SCAM_BOOL) {
            scamval_free(v);
            scamval_free(ast);
            return scamval_err("'and' passed non-boolean argument");
        } else if (!v->vals.n) {
            scamval_free(v);
            scamval_free(ast);
            return scamval_bool(0);
        } else {
            scamval_free(v);
        }
    }
    scamval_free(ast);
    return scamval_bool(1);
}

// Evaluate an or expression
scamval* eval_or(scamval* ast, scamenv* env) {
    while (scamval_len(ast) > 0) {
        scamval* v = eval(scamval_pop(ast, 0), env);
        if (v->type != SCAM_BOOL) {
            scamval_free(v);
            scamval_free(ast);
            return scamval_err("'or' passed non-boolean argument");
        } else if (v->vals.n) {
            scamval_free(v);
            scamval_free(ast);
            return scamval_bool(1);
        } else {
            scamval_free(v);
        }
    }
    scamval_free(ast);
    return scamval_bool(0);
}

// Evaluate an eval expression
scamval* eval_eval(scamval* ast, scamenv* env) {
    if (scamval_len(ast) != 2) {
        scamval_free(ast);
        return scamval_err("'eval' passed wrong number of arguments");
    } else {
        scamval* qu = eval(scamval_pop(ast, 1), env);
        scamval_free(ast);
        if (qu->type == SCAM_QUOTE) {
            qu->type = SCAM_CODE;
            return eval(qu, env);
        } else {
            scamval_free(qu);
            return scamval_err("'eval' expects quote as argument");
        }
    }
}

// Evaluate a function application
scamval* eval_apply(scamval* ast, scamenv* env) {
    // evaluate the function and the argument list
    scamval* arglist = scamval_code();
    while (scamval_len(ast) > 0) {
        scamval* v = eval(scamval_pop(ast, 0), env);
        if (v->type != SCAM_ERR) {
            scamval_append(arglist, v);
        } else {
            scamval_free(arglist);
            scamval_free(ast);
            return v;
        }
    }
    scamval_free(ast);
    scamval* fun_val = scamval_pop(arglist, 0);
    if (fun_val->type == SCAM_FUNCTION) {
        // extract the function from the scamval, for convenience
        scamfun* fun = fun_val->vals.fun;
        // make sure the right number of arguments were given
        if (scamval_len(fun->parameters) != scamval_len(arglist)) {
            scamval_free(arglist);
            return scamval_err("wrong number of arguments given");
        }
        scamenv* fun_env = scamenv_init(env);
        while (scamval_len(fun->parameters) > 0) {
            scamenv_bind(fun_env, scamval_pop(fun->parameters, 0),
                                  scamval_pop(arglist, 0));
        }
        scamval* ret = eval(fun->body, fun_env);
        // make sure there is something for scamval_free(fun_val) to free
        fun->body = scamval_null();
        scamenv_free(fun_env); 
        scamval_free(fun_val);
        scamval_free(arglist); 
        return ret;
    } else if (fun_val->type == SCAM_BUILTIN) {
        scamval* ret = fun_val->vals.bltin(arglist);
        scamval_free(fun_val); 
        scamval_free(arglist);
        return ret;
    } else {
        scamval_free(arglist); 
        scamval_free(fun_val);
        return scamval_err("first element in expression not a function");
    }
}
