#include <string.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"

#define SCAM_ASSERT(cond, ast, err, ...) { \
    if (!(cond)) { \
        return scamerr(err, ##__VA_ARGS__); \
    } \
}

#define SCAM_ASSERT_ARITY(name, ast, expected) { \
    size_t got = scamseq_len(ast); \
    if (got != expected) { \
        return scamerr_arity(name, got, expected); \
    } \
}

#define SCAM_ASSERT_MIN_ARITY(name, ast, expected) { \
    size_t got = scamseq_len(ast); \
    if (got < expected) { \
        return scamerr_arity(name, got, expected); \
    } \
}

// Forward declaration of various eval utilities
scamval* eval_begin(scamval*, scamval*);
scamval* eval_define(scamval*, scamval*);
scamval* eval_lambda(scamval*, scamval*);
scamval* eval_if(scamval*, scamval*);
scamval* eval_and(scamval*, scamval*);
scamval* eval_or(scamval*, scamval*);
scamval* eval_list(scamval*, scamval*);
scamval* eval_dict(scamval*, scamval*);

scamval* eval(scamval* ast, scamval* env) {
    if (ast->type == SCAM_SYM) {
        return scamdict_lookup(env, ast);
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
            gc_unset_root(arglist);
            ret = scamerr("first element of S-expression must be function");
        }
        gc_unset_root(arglist);
        gc_unset_root(fun_val);
        return ret;
    } else {
        return ast;
    }
}

scamval* eval_str(char* line, scamval* env) {
    scamval* ast = parse_str(line);
    scamval* ret = eval(ast, env);
    gc_unset_root(ast);
    return ret;
}

scamval* eval_file(char* fp, scamval* env) {
    scamval* ast = parse_file(fp);
    scamval* ret = eval(ast, env);
    gc_unset_root(ast);
    return ret;
}

// Evaluate a lambda expression
scamval* eval_lambda(scamval* ast, scamval* env) {
    SCAM_ASSERT_MIN_ARITY("lambda", ast, 3);
    scamval* parameters_copy = scamseq_get(ast, 1);
    SCAM_ASSERT(parameters_copy->type == SCAM_SEXPR, ast,
                "arg 1 to 'lambda' should be a parameter list");
    for (int i = 0; i < scamseq_len(parameters_copy); i++) {
        SCAM_ASSERT(scamseq_get(parameters_copy, i)->type == SCAM_SYM, ast,
                    "lambda parameter must be symbol");
    }
    return scamlambda(env, scamseq_get(ast, 1), scamseq_get(ast, 2));
}

// Evaluate a define statement
scamval* eval_define(scamval* ast, scamval* env) {
    SCAM_ASSERT_ARITY("define", ast, 3);
    SCAM_ASSERT(scamseq_get(ast, 1)->type == SCAM_SYM, ast,
                "cannot define non-symbol");
    scamval* k = scamseq_get(ast, 1);
    scamval* v = eval(scamseq_get(ast, 2), env);
    if (v->type != SCAM_ERR) {
        scamdict_bind(env, k, v);
        return scamnull();
    } else {
        return v;
    }
}

// Evaluate an if expression
scamval* eval_if(scamval* ast, scamval* env) {
    SCAM_ASSERT_ARITY("if", ast, 4);
    scamval* cond = eval(scamseq_get(ast, 1), env);
    if (cond->type == SCAM_BOOL) {
        long long cond_val = scam_as_bool(cond);
        gc_unset_root(cond);
        scamval* true_clause = scamseq_get(ast, 2);
        scamval* false_clause = scamseq_get(ast, 3);
        return cond_val ? eval(true_clause, env) : eval(false_clause, env);
    } else {
        gc_unset_root(cond);
        return scamerr("condition of an if expression must be a bool");
    }
}

// Evaluate an and expression
scamval* eval_and(scamval* ast, scamval* env) {
    size_t n = scamseq_len(ast) - 1;
    for (int i = 0; i < n; i++) {
        scamval* v = eval(scamseq_pop(ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            gc_unset_root(v);
            return scamerr_type("and", i, v_type, SCAM_BOOL);
        } else if (!scam_as_bool(v)) {
            gc_unset_root(v);
            return scambool(0);
        } else {
            gc_unset_root(v);
        }
    }
    return scambool(1);
}

// Evaluate an or expression
scamval* eval_or(scamval* ast, scamval* env) {
    size_t n = scamseq_len(ast) - 1;
    for (int i = 0; i < n; i++) {
        scamval* v = eval(scamseq_pop(ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            gc_unset_root(v);
            return scamerr_type("or", i, v_type, SCAM_BOOL);
        } else if (scam_as_bool(v)) {
            gc_unset_root(v);
            return scambool(1);
        } else {
            gc_unset_root(v);
        }
    }
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
        scamval* inner_env = scamlambda_env(fun_val);
        for (int i = 0; i < expected; i++) {
            scamdict_bind(inner_env, scamlambda_param(fun_val, i),
                          scamseq_pop(arglist, 0));
        }
        scamval* ret = eval(scamlambda_body(fun_val), inner_env);
        gc_unset_root(inner_env);
        return ret;
    } else {
        if (scambuiltin_is_const(fun_val)) {
            return scambuiltin_function(fun_val)(arglist);
        } else {
            scamval* arglist_copy = gc_copy_scamval(arglist);
            scamval* ret = scambuiltin_function(fun_val)(arglist_copy);
            gc_unset_root(arglist_copy);
            return ret;
        }
    }
}

scamval* eval_list(scamval* ast, scamval* env) {
    for (int i = 0; i < scamseq_len(ast); i++) {
        scamval* v = eval(scamseq_get(ast, i), env);
        if (v->type != SCAM_ERR) {
            gc_unset_root(v);
            scamseq_set(ast, i, v);
        } else {
            return v;
        }
    }
    return ast;
}
