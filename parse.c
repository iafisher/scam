#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "parse.h"
#include "tokenize.h"

#define TOKENIZER_MUST_ADVANCE(tz) { \
    tokenizer_advance(tz); \
    if (tz->tkn.type == TKN_EOF) { \
        return scamerr("premature end of input"); \
    } \
}

// Forward declarations of recursive descent functions
typedef scamval* (match_t)(Tokenizer*);
scamval* match_program(Tokenizer*);
scamval* match_sexpr(Tokenizer*);
scamval* match_sexpr_star(Tokenizer*);
scamval* match_sexpr_plus(Tokenizer*);
scamval* match_value(Tokenizer*);
int starts_expr(int tkn_type);
int starts_value(int tkn_type);

// Syntactic transformations
void transform_ast(scamval*);

// Parse the given input by initializing a tokenizer with the given function,
// and then calling the given matching function
scamval* parse(char* s_or_fp, tokenizer_init_t tz_init, match_t match_f) {
    Tokenizer tz; 
    tz_init(&tz, s_or_fp);
    if (tz.tkn.type == TKN_EOF) {
        tokenizer_close(&tz);
        return scamnull();
    } else {
        scamval* ret = match_f(&tz);
        transform_ast(ret);
        if (tz.tkn.type == TKN_EOF) {
            tokenizer_close(&tz);
            return ret;
        } else {
            gc_unset_root(ret);
            tokenizer_close(&tz);
            return scamerr("trailing input");
        }
    }
}

scamval* parse_str(char* s) {
    return parse(s, tokenizer_from_str, match_sexpr);
}

scamval* parse_file(char* fp) {
    return parse(fp, tokenizer_from_file, match_program);
}

scamval* match_program(Tokenizer* tz) {
    scamval* ast = match_sexpr_plus(tz);
    if (ast->type != SCAM_ERR) {
        scamseq_prepend(ast, scamsym("begin"));
        return ast;
    } else {
        return ast;
    }
}

scamval* match_sequence(Tokenizer* tz, int type, int start, int end) {
    if (tz->tkn.type == start) {
        int line = tokenizer_line(tz);
        int col = tokenizer_col(tz);
        TOKENIZER_MUST_ADVANCE(tz);
        scamval* ret = match_sexpr_star(tz);
        ret->type = type;
        if (tz->tkn.type == end) {
            tokenizer_advance(tz);
            return ret;
        } else {
            gc_unset_root(ret);
            return scamerr("expected end of expression started at line %d, "
                           "col %d", line, col);
        }
    } else {
        return scamerr("expected start of expression");
    }
}

scamval* match_sexpr(Tokenizer* tz) {
    if (tz->tkn.type == TKN_LPAREN) {
        return match_sequence(tz, SCAM_SEXPR, TKN_LPAREN, TKN_RPAREN);
    } else {
        return match_value(tz);
    }
}

scamval* match_sexpr_plus(Tokenizer* tz) {
    scamval* ast = scamsexpr();
    scamval* first_expr = match_sexpr(tz);
    if (first_expr->type != SCAM_ERR) {
        scamseq_append(ast, first_expr);
        gc_unset_root(first_expr);
    } else {
        gc_unset_root(ast);
        return first_expr;
    }
    while (starts_expr(tz->tkn.type)) {
        scamval* next_expr = match_sexpr(tz);
        scamseq_append(ast, next_expr);
        gc_unset_root(next_expr);
    }
    return ast;
}

scamval* match_sexpr_star(Tokenizer* tz) {
    scamval* ast = scamsexpr();
    while (starts_expr(tz->tkn.type)) {
        scamval* next_expr = match_sexpr(tz);
        scamseq_append(ast, next_expr);
        gc_unset_root(next_expr);
    }
    return ast;
}

scamval* match_value(Tokenizer* tz) {
    if (starts_value(tz->tkn.type)) {
        if (tz->tkn.type == TKN_LBRACE) {
            return scamerr("brace expressions currently not implemented");
        } else if (tz->tkn.type == TKN_LBRACKET) {
            return match_sequence(tz, SCAM_LIST, TKN_LBRACKET, TKN_RBRACKET);
        } else {
            scamval* ret = scamval_from_token(&tz->tkn);
            ret->line = tokenizer_line(tz);
            ret->col = tokenizer_col(tz);
            tokenizer_advance(tz);
            return ret;
        }
    } else {
        return scamerr("expected start of value");
    }
}

scamval* scamval_from_token(Token* tkn) {
    switch (tkn->type) {
        case TKN_INT: return scamint(strtoll(tkn->val, NULL, 10));
        case TKN_DEC: return scamdec(strtod(tkn->val, NULL));
        case TKN_SYM: 
            if (strcmp(tkn->val, "true") == 0) {
                return scambool(1);
            } else if (strcmp(tkn->val, "false") == 0) {
                return scambool(0);
            } else {
                return scamsym(tkn->val);
            }
        case TKN_STR:
            {
                // remove the quotes from string literals
                scamval* ret = scamstr(tkn->val + 1);
                ret->vals.s[ret->count - 1] = '\0';
                ret->count--;
                return ret;
            }
        default: return scamerr("unknown token type");
    }
}

int starts_expr(int tkn_type) {
    return tkn_type == TKN_LPAREN || starts_value(tkn_type);
}

int starts_value(int tkn_type) {
    return tkn_type == TKN_LBRACKET || tkn_type == TKN_LBRACE ||
           tkn_type == TKN_INT || tkn_type == TKN_DEC || tkn_type == TKN_STR ||
           tkn_type == TKN_SYM;
}

typedef int (transform_pred_t)(scamval*);
typedef void (transform_func_t)(scamval*);

int transform_define_pred(scamval* ast) {
    if (scamseq_len(ast) >= 3) {
        scamval* first = scamseq_get(ast, 0);
        scamval* second = scamseq_get(ast, 1);
        if (first->type == SCAM_SYM && strcmp(first->vals.s, "define") == 0) {
            if (second->type == SCAM_SEXPR && scamseq_len(second) >= 1) {
                return 1;
            }
        }
    }
    return 0;
}

void transform_define(scamval* ast) {
    // ast == (define (...) body)
    scamval* parameters = scamseq_pop(ast, 1);
    // ast == (define body)
    scamval* name = scamseq_pop(parameters, 0);
    scamval* lambda_body = scamsexpr();
    scamseq_append(lambda_body, scamsym("begin"));
    // lambda == (lambda (...))
    while (scamseq_len(ast) > 1)
        scamseq_append(lambda_body, scamseq_pop(ast, 1));
    // ast == (define)
    scamval* lambda = scamsexpr_from_vals(3, scamsym("lambda"), parameters,
                                             lambda_body);
    // lambda == (lambda (...) body)
    scamseq_append(ast, name);
    scamseq_append(ast, lambda);
}

int transform_and_pred(scamval* ast) {
    if (scamseq_len(ast) == 3) {
        scamval* first = scamseq_get(ast, 0);
        if (first->type == SCAM_SYM && strcmp(first->vals.s, "and") == 0) {
            return 1;
        }
    }
    return 0;
}

// (and cond1 cond2) (if cond1 cond2 false)
// (and cond1 cond2 cond3) (if cond1 (and cond2 cond3) false)
void transform_and(scamval* ast) {
    // ast == (and cond1 cond2)
    scamseq_delete(ast, 0);
    // ast == (cond1 cond2)
    scamseq_prepend(ast, scamsym("if"));
    // ast == (if cond1 cond2)
    scamseq_append(ast, scambool(0));
    // ast == (if cond1 cond2 false)
}

void do_transform(scamval* ast, transform_pred_t pred, transform_func_t func) {
    if (ast->type == SCAM_SEXPR) {
        if (pred(ast)) {
            func(ast);
        }
        for (int i = 0; i < scamseq_len(ast); i++) {
            do_transform(scamseq_get(ast, i), pred, func);
        }
    }
}

void transform_ast(scamval* ast) {
    do_transform(ast, transform_define_pred, transform_define);
    //do_transform(ast, transform_and_pred, transform_and);
}
