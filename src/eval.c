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
    size_t got = ScamSeq_len((ScamSeq*)ast); \
    if (got != expected) { \
        return (ScamVal*)ScamErr_arity(name, got, expected); \
    } \
}

#define SCAM_ASSERT_MIN_ARITY(name, ast, expected) { \
    size_t got = ScamSeq_len((ScamSeq*)ast); \
    if (got < expected) { \
        return (ScamVal*)ScamErr_arity(name, got, expected); \
    } \
}

// Forward declaration of various eval utilities
ScamVal* eval_begin(ScamVal*, ScamDict*);
ScamVal* eval_define(ScamVal*, ScamDict*);
ScamVal* eval_lambda(ScamVal*, ScamDict*);
ScamVal* eval_if(ScamVal*, ScamDict*);
ScamVal* eval_and(ScamVal*, ScamDict*);
ScamVal* eval_or(ScamVal*, ScamDict*);
ScamList* eval_list(ScamList*, ScamDict*);
ScamDict* eval_dict(ScamVal*, ScamDict*);

ScamVal* eval(ScamVal* ast, ScamDict* env) {
    if (ast->type == SCAM_SYM) {
        return ScamDict_lookup(env, (ScamSym*)ast);
    } else if (ast->type == SCAM_SEXPR) {
        SCAM_ASSERT(ScamSeq_len((ScamSeq*)ast) > 0, ast, "empty expression");
        // handle special expressions and statements
        if (ScamSeq_get((ScamSeq*)ast, 0)->type == SCAM_SYM) {
            const char* name = ScamStr_unbox((ScamStr*)ScamSeq_get((ScamSeq*)ast, 0));
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
        ScamList* arglist = eval_list((ScamList*)ast, env);
        if (arglist->type == SCAM_ERR) {
            return (ScamVal*)arglist;
        }
        ScamVal* fun_val = ScamSeq_pop((ScamSeq*)arglist, 0);
        ScamVal* ret;
        if (ScamVal_typecheck(fun_val, SCAM_FUNCTION)) {
            ret = eval_apply(fun_val, arglist);
        } else {
            gc_unset_root((ScamVal*)arglist);
            ret = (ScamVal*)ScamErr_new("first element of S-expression must be function");
        }
        gc_unset_root((ScamVal*)arglist);
        gc_unset_root(fun_val);
        return ret;
    } else {
        return ast;
    }
}

ScamVal* eval_str(char* line, ScamDict* env) {
    ScamExpr* ast = parse_str(line);
    ScamVal* ret = eval((ScamVal*)ast, env);
    gc_unset_root((ScamVal*)ast);
    return ret;
}

ScamVal* eval_file(char* fp, ScamDict* env) {
    ScamExpr* ast = parse_file(fp);
    ScamVal* ret = eval((ScamVal*)ast, env);
    gc_unset_root((ScamVal*)ast);
    return ret;
}

// Evaluate a lambda expression
ScamVal* eval_lambda(ScamVal* ast, ScamDict* env) {
    SCAM_ASSERT_MIN_ARITY("lambda", ast, 3);
    ScamVal* parameters_copy = ScamSeq_get((ScamSeq*)ast, 1);
    SCAM_ASSERT(parameters_copy->type == SCAM_SEXPR, ast,
                "arg 1 to 'lambda' should be a parameter list");
    for (int i = 0; i < ScamSeq_len((ScamSeq*)parameters_copy); i++) {
        SCAM_ASSERT(ScamSeq_get((ScamSeq*)parameters_copy, i)->type == SCAM_SYM, ast,
                    "lambda parameter must be symbol");
    }
    return (ScamVal*)ScamFunction_new(env, (ScamList*)ScamSeq_get((ScamSeq*)ast, 1), 
                                           (ScamExpr*)ScamSeq_get((ScamSeq*)ast, 2));
}

// Evaluate a define statement
ScamVal* eval_define(ScamVal* ast, ScamDict* env) {
    SCAM_ASSERT_ARITY("define", ast, 3);
    SCAM_ASSERT(ScamSeq_get((ScamSeq*)ast, 1)->type == SCAM_SYM, ast, "cannot define non-symbol");
    ScamSym* k = (ScamSym*)ScamSeq_get((ScamSeq*)ast, 1);
    ScamVal* v = eval(ScamSeq_get((ScamSeq*)ast, 2), env);
    if (v->type != SCAM_ERR) {
        ScamDict_bind(env, k, v);
        return ScamNull_new();
    } else {
        return v;
    }
}

// Evaluate an if expression
ScamVal* eval_if(ScamVal* ast, ScamDict* env) {
    SCAM_ASSERT_ARITY("if", ast, 4);
    ScamVal* cond = eval(ScamSeq_get((ScamSeq*)ast, 1), env);
    if (cond->type == SCAM_BOOL) {
        long long cond_val = ScamBool_unbox((ScamInt*)cond);
        gc_unset_root(cond);
        ScamVal* true_clause = ScamSeq_get((ScamSeq*)ast, 2);
        ScamVal* false_clause = ScamSeq_get((ScamSeq*)ast, 3);
        return cond_val ? eval(true_clause, env) : eval(false_clause, env);
    } else {
        gc_unset_root(cond);
        return (ScamVal*)ScamErr_new("condition of an if expression must be a bool");
    }
}

// Evaluate an and expression
ScamVal* eval_and(ScamVal* ast, ScamDict* env) {
    size_t n = ScamSeq_len((ScamSeq*)ast) - 1;
    for (int i = 0; i < n; i++) {
        ScamVal* v = eval(ScamSeq_pop((ScamSeq*)ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            gc_unset_root(v);
            return (ScamVal*)ScamErr_type("and", i, v_type, SCAM_BOOL);
        } else if (!ScamBool_unbox((ScamInt*)v)) {
            gc_unset_root(v);
            return (ScamVal*)ScamBool_new(0);
        } else {
            gc_unset_root(v);
        }
    }
    return (ScamVal*)ScamBool_new(1);
}

// Evaluate an or expression
ScamVal* eval_or(ScamVal* ast, ScamDict* env) {
    size_t n = ScamSeq_len((ScamSeq*)ast) - 1;
    for (int i = 0; i < n; i++) {
        ScamVal* v = eval(ScamSeq_pop((ScamSeq*)ast, 1), env);
        int v_type = v->type;
        if (v_type != SCAM_BOOL) {
            gc_unset_root(v);
            return (ScamVal*)ScamErr_type("or", i, v_type, SCAM_BOOL);
        } else if (ScamBool_unbox((ScamInt*)v)) {
            gc_unset_root(v);
            return (ScamVal*)ScamBool_new(1);
        } else {
            gc_unset_root(v);
        }
    }
    return (ScamVal*)ScamBool_new(0);
}


ScamVal* eval_apply(ScamVal* fun_val, ScamList* arglist) {
    if (fun_val->type == SCAM_LAMBDA) {
        ScamFunction* lamb = (ScamFunction*)fun_val;
        // make sure the right number of arguments were given
        size_t expected = ScamFunction_nparams(lamb);
        size_t got = ScamSeq_len((ScamSeq*)arglist);
        if (got != expected) {
            return (ScamVal*)ScamErr_new("lambda function got %d argument(s), expected %d", 
                                         got, expected);
        }
        ScamDict* inner_env = ScamFunction_env((ScamFunction*)fun_val);
        for (int i = 0; i < expected; i++) {
            ScamDict_bind(inner_env, ScamFunction_param(lamb, i), ScamSeq_pop((ScamSeq*)arglist, 0));
        }
        ScamVal* ret = eval((ScamVal*)ScamFunction_body(lamb), inner_env);
        gc_unset_root((ScamVal*)inner_env);
        return ret;
    } else {
        if (ScamBuiltin_is_const((ScamBuiltin*)fun_val)) {
            return ScamBuiltin_function((ScamBuiltin*)fun_val)(arglist);
        } else {
            ScamList* arglist_copy = (ScamList*)gc_copy_ScamVal((ScamVal*)arglist);
            ScamVal* ret = ScamBuiltin_function((ScamBuiltin*)fun_val)(arglist_copy);
            gc_unset_root((ScamVal*)arglist_copy);
            return ret;
        }
    }
}

ScamList* eval_list(ScamList* ast, ScamDict* env) {
    for (int i = 0; i < ScamSeq_len((ScamSeq*)ast); i++) {
        ScamVal* v = eval(ScamSeq_get((ScamSeq*)ast, i), env);
        if (v->type != SCAM_ERR) {
            gc_unset_root(v);
            ScamSeq_set((ScamSeq*)ast, i, v);
        } else {
            return (ScamList*)v;
        }
    }
    return (ScamList*)ast;
}
