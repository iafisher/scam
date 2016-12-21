#include <string.h>
#include "eval.h"
#include "parse.h"
#include "scamtypes.h"

#define SCAM_ASSERT(cond, ast, err, ...) { \
    if (!(cond)) { \
        scamval_free(ast); \
        return scamerr(err, ##__VA_ARGS__); \
    } \
}

#define SCAM_ASSERT_ARITY(name, ast, expected) { \
    size_t got = scamseq_len(ast); \
    if (got != expected) { \
        scamval_free(ast); \
        return scamerr_arity(name, got, expected); \
    } \
}

#define SCAM_ASSERT_MIN_ARITY(name, ast, expected) { \
    size_t got = scamseq_len(ast); \
    if (got < expected) { \
        scamval_free(ast); \
        return scamerr_arity(name, got, expected); \
    } \
}

// Forward declaration of various eval utilities
scamval* eval_define(scamval*, scamenv*);
scamval* eval_lambda(scamval*, scamenv*);
scamval* eval_if(scamval*, scamenv*);
scamval* eval_and(scamval*, scamenv*);
scamval* eval_or(scamval*, scamenv*);
scamval* eval_list(scamval*, scamenv*);

scamval* eval(scamval* ast, scamenv* env) {
    if (ast->type == SCAM_SYM) {
        scamval* ret = scamenv_lookup(env, ast);
        scamval_free(ast);
        return ret;
    } else if (ast->type == SCAM_SEXPR) {
        SCAM_ASSERT(scamseq_len(ast) > 0, ast, "empty expression");
        // handle special expressions and statements
        if (scamseq_get(ast, 0)->type == SCAM_SYM) {
            char* name = scamseq_get(ast, 0)->vals.s;
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
            }
        }
        scamval* arglist = eval_list(ast, env);
        if (arglist->type == SCAM_ERR) {
            return arglist;
        }
        scamval* fun_val = scamseq_pop(arglist, 0);
        scamval* ret;
        if (scamval_typecheck(fun_val, SCAM_FUNCTION)) {
            ret = eval_apply(fun_val, arglist);
        } else {
            ret = scamerr("first element of S-expression must be function");
        }
        scamval_free(arglist);
        scamval_free(fun_val);
        return ret;
    } else if (ast->type == SCAM_LIST) {
        return eval_list(ast, env);
    } else {
        return ast;
    }
}

scamval* eval_str(char* line, scamenv* env) {
    return eval(parse_str(line), env);
}

scamval* eval_file(char* fp, scamenv* env) {
    return eval(parse_file(fp), env);
}

// Evaluate a lambda expression
scamval* eval_lambda(scamval* ast, scamenv* env) {
    SCAM_ASSERT_MIN_ARITY("lambda", ast, 3);
    scamval* parameters_copy = scamseq_get(ast, 1);
    SCAM_ASSERT(parameters_copy->type == SCAM_SEXPR, ast,
                "arg 1 to 'lambda' should be a parameter list");
    for (int i = 0; i < scamseq_len(parameters_copy); i++) {
        SCAM_ASSERT(scamseq_get(parameters_copy, i)->type == SCAM_SYM, ast,
                    "lambda parameter must be symbol");
    }
    // remove and free the 'lambda' symbol
    scamval_free(scamseq_pop(ast, 0));
    scamval* parameters = scamseq_pop(ast, 0);
    scamval* body = scamseq_pop(ast, 0);
    scamval_free(ast);
    return scamlambda(env, parameters, body);
}

// Evaluate a define statement
scamval* eval_define(scamval* ast, scamenv* env) {
    SCAM_ASSERT_ARITY("define", ast, 3);
    SCAM_ASSERT(scamseq_get(ast, 1)->type == SCAM_SYM, ast,
                "cannot define non-symbol");
    scamval* k = scamseq_pop(ast, 1);
    scamval* v = eval(scamseq_pop(ast, 1), env);
    scamval_free(ast);
    if (v->type != SCAM_ERR) {
        scamenv_bind(env, k, v);
        return scamnull();
    } else {
        scamval_free(k);
        return v;
    }
}

// Evaluate an if expression
scamval* eval_if(scamval* ast, scamenv* env) {
    SCAM_ASSERT_ARITY("if", ast, 4);
    scamval* cond = eval(scamseq_pop(ast, 1), env);
    if (cond->type == SCAM_BOOL) {
        scamval* true_clause = scamseq_pop(ast, 1);
        scamval* false_clause = scamseq_pop(ast, 1);
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
        return scamerr("condition of an if expression must be a bool");
    }
}

// Evaluate an and expression
scamval* eval_and(scamval* ast, scamenv* env) {
    size_t n = scamseq_len(ast) - 1;
    for (int i = 0; i < n; i++) {
        scamval* v = eval(scamseq_pop(ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            scamval_free(v);
            scamval_free(ast);
            return scamerr_type("and", i, v_type, SCAM_BOOL);
        } else if (!v->vals.n) {
            scamval_free(v);
            scamval_free(ast);
            return scambool(0);
        } else {
            scamval_free(v);
        }
    }
    scamval_free(ast);
    return scambool(1);
}

// Evaluate an or expression
scamval* eval_or(scamval* ast, scamenv* env) {
    size_t n = scamseq_len(ast) - 1;
    for (int i = 0; i < n; i++) {
        scamval* v = eval(scamseq_pop(ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            scamval_free(v);
            scamval_free(ast);
            return scamerr_type("or", i, v_type, SCAM_BOOL);
        } else if (v->vals.n) {
            scamval_free(v);
            scamval_free(ast);
            return scambool(1);
        } else {
            scamval_free(v);
        }
    }
    scamval_free(ast);
    return scambool(0);
}

// Evaluate a function application
scamval* eval_apply(scamval* fun_val, scamval* arglist) {
    if (fun_val->type == SCAM_LAMBDA) {
        // make sure the right number of arguments were given
        size_t expected = scamlambda_nparams(fun_val);
        size_t got = scamseq_len(arglist);
        if (got != expected) {
            return scamerr("lambda function got %d argument(s), expected %d", 
                           got, expected);
        }
        scamenv* inner_env = scamlambda_env(fun_val);
        for (int i = 0; i < expected; i++) {
            scamenv_bind(inner_env, scamlambda_param(fun_val, i),
                         scamseq_pop(arglist, 0));
        }
        scamval* ret = eval(scamlambda_body(fun_val), inner_env);
        scamenv_free(inner_env);
        return ret;
    } else {
        if (scambuiltin_is_const(fun_val)) {
            scamval* ret = scambuiltin_function(fun_val)(arglist);
            return ret;
        } else {
            scamval* arglist_copy = scamval_copy(arglist);
            scamval* ret = scambuiltin_function(fun_val)(arglist_copy);
            scamval_free(arglist_copy);
            return ret;
        }
    }
}

scamval* eval_list(scamval* ast, scamenv* env) {
    for (int i = 0; i < scamseq_len(ast); i++) {
        scamval* v = eval(scamseq_get(ast, i), env);
        if (v->type != SCAM_ERR) {
            scamseq_set(ast, i, v);
        } else {
            // the i'th node was freed by eval, so it must be replaced by an
            // allocated value
            scamseq_set(ast, i, scamnull());
            scamval_free(ast);
            return v;
        }
    }
    return ast;
}
