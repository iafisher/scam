#include <string.h>
#include "eval.h"
#include "parse.h"

#define SCAM_ASSERT(cond, ast, err, ...) { \
    if (!(cond)) { \
        scamval_free(ast); \
        return scamval_err(err, ##__VA_ARGS__); \
    } \
}

#define SCAM_ASSERT_ARITY(name, ast, n) { \
    size_t this_n = scamval_len(ast); \
    if (this_n != n) { \
        scamval_free(ast); \
        return scamval_err("'%s' expected %d argument(s), got %d", name, n, this_n); \
    } \
}

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
        SCAM_ASSERT(scamval_len(ast) > 0, ast, "empty expression");
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
    SCAM_ASSERT_ARITY("lambda", ast, 3);
    scamval* parameters_copy = scamval_get(ast, 1);
    SCAM_ASSERT(parameters_copy->type == SCAM_CODE, ast,
                "arg 2 to 'lambda' should be a parameter list");
    for (int i = 0; i < scamval_len(parameters_copy); i++) {
        SCAM_ASSERT(scamval_get(parameters_copy, i)->type == SCAM_SYM, ast,
                    "lambda parameter must be symbol");
    }
    scamval* parameters = scamval_pop(ast, 1);
    scamval* body = scamval_pop(ast, 1);
    scamval_free(ast);
    return scamval_function(env, parameters, body);
}

scamval* eval_define_val(scamval* ast, scamenv* env) {
    SCAM_ASSERT_ARITY("define", ast, 3);
    SCAM_ASSERT(scamval_get(ast, 1)->type == SCAM_SYM, ast,
                "cannot define non-symbol");
    scamval* k = scamval_pop(ast, 1);
    scamval* v = eval(scamval_pop(ast, 1), env);
    scamval_free(ast);
    if (v->type != SCAM_ERR) {
        scamenv_bind(env, k, v);
        return scamval_null();
    } else {
        scamval_free(k);
        return v;
    }
}

scamval* eval_define_fun(scamval* ast, scamenv* env) {
    SCAM_ASSERT(scamval_get(ast, 1)->type == SCAM_CODE, ast,
                "argument 1 of define should be a symbol");
    scamval* par_list = scamval_get(ast, 1);
    SCAM_ASSERT(scamval_len(par_list) > 0, ast, "empty expression");
    for (int i = 0; i < scamval_len(par_list); i++) {
        SCAM_ASSERT(scamval_get(par_list, i)->type == SCAM_SYM, ast,
                    "function name or parameter must be a symbol");
    }
    // replace the copy of the parameter list with the actual thing
    par_list = scamval_pop(ast, 1);
    scamval* body = scamval_pop(ast, 1);
    scamval_free(ast);
    scamval* f_name = scamval_pop(par_list, 0);
    scamval* fun = scamval_function(env, par_list, body);
    scamenv_bind(env, f_name, fun);
    return scamval_null();
}

// Evaluate a define statement
scamval* eval_define(scamval* ast, scamenv* env) {
    SCAM_ASSERT_ARITY("define", ast, 3);
    if (scamval_get(ast, 1)->type == SCAM_SYM) {
        return eval_define_val(ast, env);
    } else {
        return eval_define_fun(ast, env);
    }
}

// Evaluate an if expression
scamval* eval_if(scamval* ast, scamenv* env) {
    SCAM_ASSERT_ARITY("if", ast, 4);
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
    SCAM_ASSERT_ARITY("eval", ast, 2);
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
    scamval* fun_val_copy = scamval_get(arglist, 0);
    if (fun_val_copy->type == SCAM_FUNCTION) {
        // extract the function from the scamval, for convenience
        scamfun* fun = fun_val_copy->vals.fun;
        // make sure the right number of arguments were given
        size_t expected = scamval_len(fun->parameters);
        size_t got = scamval_len(arglist) - 1;
        SCAM_ASSERT(got == expected, arglist, 
                    "'lambda' got %d argument(s), expected %d", got, expected);
        scamenv* fun_env = scamenv_init(env);
        while (scamval_len(fun->parameters) > 0) {
            scamenv_bind(fun_env, scamval_pop(fun->parameters, 0),
                                  scamval_pop(arglist, 1));
        }
        scamval* ret = eval(fun->body, fun_env);
        // make sure there is something for scamval_free(fun_val) to free
        fun->body = scamval_null();
        scamenv_free(fun_env); 
        scamval_free(arglist); 
        return ret;
    } else if (fun_val_copy->type == SCAM_BUILTIN) {
        // pop the function off the arglist so it contains only the arguments
        scamval* fun_val = scamval_pop(arglist, 0);
        scamval* ret = fun_val->vals.bltin(arglist);
        scamval_free(fun_val); 
        scamval_free(arglist);
        return ret;
    } else {
        scamval_free(arglist); 
        return scamval_err("first element in expression not a function");
    }
}
