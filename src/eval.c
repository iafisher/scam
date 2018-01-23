#include <string.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"

#define SCAM_ASSERT(cond, ast, err, ...) { \
    if (!(cond)) { \
        return (ScamVal*)ScamErr_new(err, ##__VA_ARGS__); \
    } \
}

#define SCAM_ASSERT_ARITY(name, ast, expected) { \
    size_t got = ScamSeq_len(ast); \
    if (got != expected) { \
        return (ScamVal*)ScamErr_arity(name, got, expected); \
    } \
}

#define SCAM_ASSERT_MIN_ARITY(name, ast, expected) { \
    size_t got = ScamSeq_len(ast); \
    if (got < expected) { \
        return (ScamVal*)ScamErr_arity(name, got, expected); \
    } \
}

/* Forward declaration of various eval utilities */
ScamVal* eval_begin(ScamSeq*, ScamEnv*);
ScamVal* eval_define(ScamSeq*, ScamEnv*);
ScamVal* eval_lambda(ScamSeq*, ScamEnv*);
ScamVal* eval_if(ScamSeq*, ScamEnv*);
ScamVal* eval_and(ScamSeq*, ScamEnv*);
ScamVal* eval_or(ScamSeq*, ScamEnv*);
ScamSeq* eval_list(ScamSeq*, ScamEnv*);
ScamDict* eval_dict(ScamSeq*, ScamEnv*);

ScamVal* eval(ScamVal* ast_or_val, ScamEnv* env) {
    if (ast_or_val->type == SCAM_SYM) {
        return ScamEnv_lookup(env, (ScamStr*)ast_or_val);
    } else if (ast_or_val->type == SCAM_SEXPR) {
        ScamSeq* ast = (ScamSeq*)ast_or_val;
        SCAM_ASSERT(ScamSeq_len(ast) > 0, ast, "empty expression");
        /* Handle special expressions and statements. */
        if (ScamSeq_get(ast, 0)->type == SCAM_SYM) {
            const char* name = ScamStr_unbox((ScamStr*)ScamSeq_get(ast, 0));
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
        ScamSeq* arglist = eval_list(ast, env);
        if (arglist->type == SCAM_ERR) {
            return (ScamVal*)arglist;
        }
        ScamVal* fun_val = ScamSeq_pop(arglist, 0);
        ScamVal* ret;
        if (ScamVal_typecheck(fun_val, SCAM_BASE_FUNCTION)) {
            ret = eval_apply(fun_val, arglist);
        } else {
            gc_unset_root((ScamVal*)arglist);
            ret = (ScamVal*)ScamErr_new("first element of S-expression must be function");
        }
        gc_unset_root((ScamVal*)arglist);
        gc_unset_root(fun_val);
        return ret;
    } else {
        return ast_or_val;
    }
}

ScamVal* eval_str(char* line, ScamEnv* env) {
    ScamSeq* ast = parse_str(line);
    ScamVal* ret = eval((ScamVal*)ast, env);
    gc_unset_root((ScamVal*)ast);
    return ret;
}

ScamVal* eval_file(char* fp, ScamEnv* env) {
    ScamSeq* ast = parse_file(fp);
    ScamVal* ret = eval((ScamVal*)ast, env);
    gc_unset_root((ScamVal*)ast);
    return ret;
}

/* Evaluate a lambda expression. */
ScamVal* eval_lambda(ScamSeq* ast, ScamEnv* env) {
    SCAM_ASSERT_MIN_ARITY("lambda", ast, 3);
    ScamSeq* parameters_copy = (ScamSeq*)ScamSeq_get(ast, 1);
    SCAM_ASSERT(parameters_copy->type == SCAM_SEXPR, ast,
                "arg 1 to 'lambda' should be a parameter list");
    for (size_t i = 0; i < ScamSeq_len(parameters_copy); i++) {
        SCAM_ASSERT(ScamSeq_get(parameters_copy, i)->type == SCAM_SYM, ast,
                    "lambda parameter must be symbol");
    }
    return (ScamVal*)ScamFunction_new(env, (ScamSeq*)ScamSeq_get(ast, 1),
                                           (ScamSeq*)ScamSeq_get(ast, 2));
}

/* Evaluate a define statement. */
ScamVal* eval_define(ScamSeq* ast, ScamEnv* env) {
    SCAM_ASSERT_ARITY("define", ast, 3);
    SCAM_ASSERT(ScamSeq_get(ast, 1)->type == SCAM_SYM, ast, "cannot define non-symbol");
    ScamVal* k = ScamSeq_get(ast, 1);
    ScamVal* v = eval(ScamSeq_get(ast, 2), env);
    if (v->type != SCAM_ERR) {
        ScamEnv_insert(env, (ScamStr*)k, v);
        return ScamNull_new();
    } else {
        return v;
    }
}

/* Evaluate an if expression. */
ScamVal* eval_if(ScamSeq* ast, ScamEnv* env) {
    SCAM_ASSERT_ARITY("if", ast, 4);
    ScamVal* cond = eval(ScamSeq_get(ast, 1), env);
    if (cond->type == SCAM_BOOL) {
        long long cond_val = ScamBool_unbox((ScamBool*)cond);
        gc_unset_root(cond);
        ScamVal* true_clause = ScamSeq_get(ast, 2);
        ScamVal* false_clause = ScamSeq_get(ast, 3);
        return cond_val ? eval(true_clause, env) : eval(false_clause, env);
    } else {
        gc_unset_root(cond);
        return (ScamVal*)ScamErr_new("condition of an if expression must be a bool");
    }
}

/* Evaluate an and expression. */
ScamVal* eval_and(ScamSeq* ast, ScamEnv* env) {
    size_t n = ScamSeq_len(ast) - 1;
    for (size_t i = 0; i < n; i++) {
        ScamVal* v = eval(ScamSeq_pop(ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            gc_unset_root(v);
            return (ScamVal*)ScamErr_type("and", i, v_type, SCAM_BOOL);
        } else if (!ScamBool_unbox((ScamBool*)v)) {
            gc_unset_root(v);
            return (ScamVal*)ScamBool_new(0);
        } else {
            gc_unset_root(v);
        }
    }
    return (ScamVal*)ScamBool_new(1);
}

/* Evaluate an or expression. */
ScamVal* eval_or(ScamSeq* ast, ScamEnv* env) {
    size_t n = ScamSeq_len(ast) - 1;
    for (size_t i = 0; i < n; i++) {
        ScamVal* v = eval(ScamSeq_pop(ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            gc_unset_root(v);
            return (ScamVal*)ScamErr_type("or", i, v_type, SCAM_BOOL);
        } else if (ScamBool_unbox((ScamBool*)v)) {
            gc_unset_root(v);
            return (ScamVal*)ScamBool_new(1);
        } else {
            gc_unset_root(v);
        }
    }
    return (ScamVal*)ScamBool_new(0);
}


ScamVal* eval_apply(ScamVal* fun_val, ScamSeq* arglist) {
    if (fun_val->type == SCAM_FUNCTION) {
        ScamFunction* lamb = (ScamFunction*)fun_val;
        /* Make sure the right number of arguments were given. */
        size_t expected = ScamFunction_nparams(lamb);
        size_t got = ScamSeq_len(arglist);
        if (got != expected) {
            return (ScamVal*)ScamErr_new("lambda function got %d argument(s), expected %d",
                                         got, expected);
        }
        ScamEnv* inner_env = ScamFunction_env((ScamFunction*)fun_val);
        for (size_t i = 0; i < expected; i++) {
            ScamEnv_insert(inner_env, ScamFunction_param(lamb, i), ScamSeq_pop(arglist, 0));
        }
        ScamVal* ret = eval((ScamVal*)ScamFunction_body(lamb), inner_env);
        gc_unset_root((ScamVal*)inner_env);
        return ret;
    } else {
        if (ScamBuiltin_is_const((ScamBuiltin*)fun_val)) {
            return ScamBuiltin_function((ScamBuiltin*)fun_val)(arglist);
        } else {
            ScamSeq* arglist_copy = (ScamSeq*)gc_copy_ScamVal((ScamVal*)arglist);
            ScamVal* ret = ScamBuiltin_function((ScamBuiltin*)fun_val)(arglist_copy);
            gc_unset_root((ScamVal*)arglist_copy);
            return ret;
        }
    }
}

ScamSeq* eval_list(ScamSeq* ast, ScamEnv* env) {
    for (size_t i = 0; i < ScamSeq_len(ast); i++) {
        ScamVal* v = eval(ScamSeq_get(ast, i), env);
        if (v->type != SCAM_ERR) {
            gc_unset_root(v);
            ScamSeq_set(ast, i, v);
        } else {
            return (ScamSeq*)v;
        }
    }
    return (ScamSeq*)ast;
}
