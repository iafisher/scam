#include "parse.h"
#include "tokenize.h"

// Forward declarations of recursive descent functions
scamval* match_expr(Tokenizer*);
scamval* match_expr_plus(Tokenizer*);
scamval* match_value(Tokenizer*);
int starts_expr(int tkn_type);
int starts_value(int tkn_type);
scamval* scamval_from_token(Token*);

scamval* parse_line(char* s) {
    Tokenizer* tz = tokenizer_from_str(s);
    scamval* ret = match_expr(tz);
    tokenizer_close(tz);
    return ret;
}

scamval* parse_file(char* fp) {
    Tokenizer* tz = tokenizer_from_file(fp);
    scamval* ret = match_expr_plus(tz);
    tokenizer_close(tz);
    return ret;
}

scamval* match_any_expr(Tokenizer* tz, int type, int start, int end) {
    if (tz->tkn->type == start) {
        tokenizer_advance(tz);
        scamval* ret = match_expr_plus(tz);
        ret->type = type;
        if (tz->tkn->type == end) {
            tokenizer_advance(tz);
            return ret;
        } else {
            scamval_free(ret);
            return scamval_err("expected end of expression");
        }
    } else {
        return scamval_err("expected start of expression");
    }
}

scamval* match_any_nonempty_expr(Tokenizer* tz, int type, int start, int end) {
    scamval* ret = match_any_expr(tz, type, start, end);
    if (ret->vals.arr->count > 0) {
        return ret;
    } else {
        scamval_free(ret);
        return scamval_err("empty expression");
    }
}

scamval* match_expr(Tokenizer* tz) {
    if (tz->tkn->type == TKN_LPAREN) {
        return match_any_nonempty_expr(tz, SCAM_CODE, TKN_LPAREN, TKN_RPAREN);
    } else {
        return match_value(tz);
    }
}

scamval* match_expr_plus(Tokenizer* tz) {
    scamval* ast = scamval_code();
    scamval* first_expr = match_expr(tz);
    if (first_expr->type != SCAM_ERR) {
        scamval_append(ast, first_expr);
    } else {
        scamval_free(ast);
        return first_expr;
    }
    while (starts_expr(tz->tkn->type))
        scamval_append(ast, match_expr(tz));
    return ast;
}

scamval* match_expr_star(Tokenizer* tz) {
    scamval* ast = scamval_code();
    while (starts_expr(tz->tkn->type))
        scamval_append(ast, match_expr(tz));
    return ast;
}

scamval* match_value(Tokenizer* tz) {
    if (starts_value(tz->tkn->type)) {
        if (tz->tkn->type == TKN_RBRACE) {
            return match_any_expr(tz, SCAM_QUOTE, TKN_LBRACE, TKN_RBRACE);
        } else if (tz->tkn->type == TKN_RBRACKET) {
            return match_any_expr(tz, SCAM_LIST, TKN_LBRACKET, TKN_RBRACKET);
        } else {
            scamval* ret = scamval_from_token(tz->tkn);
            tokenizer_advance(tz);
            return ret;
        }
    } else {
        return scamval_err("expected start of value");
    }
}

scamval* match_value_star(Tokenizer* tz) {
    scamval* ast = scamval_code();
    scamval* first_val = match_value(tz);
    if (first_val->type != SCAM_ERR) {
        scamval_append(ast, first_val);
    } else {
        scamval_free(ast);
        return first_val;
    }
    while (starts_value(tz->tkn->type))
        scamval_append(ast, match_value(tz));
    return ast;
}

scamval* scamval_from_token(Token* tkn) {
    // this will need to be changed
    return scamval_sym(tkn->val);
}

int starts_expr(int tkn_type) {
    return tkn_type == TKN_LPAREN || starts_value(tkn_type);
}

int starts_value(int tkn_type) {
    return tkn_type == TKN_LBRACKET || tkn_type == TKN_LBRACE ||
           tkn_type == TKN_INT || tkn_type == TKN_DEC || tkn_type == TKN_STR ||
           tkn_type == TKN_SYM;
}
