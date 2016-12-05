#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "tokenize.h"

#define TOKENIZER_MUST_ADVANCE(tz) { \
    tokenizer_advance(tz); \
    if (tz->tkn->type == TKN_EOF) { \
        return scamval_err("premature end of input"); \
    } \
}

// Forward declarations of recursive descent functions
scamval* match_expr(Tokenizer*);
scamval* match_expr_star(Tokenizer*);
scamval* match_expr_plus(Tokenizer*);
scamval* match_value(Tokenizer*);
int starts_expr(int tkn_type);
int starts_value(int tkn_type);
scamval* scamval_from_token(Token*);

scamval* parse_line(char* s) {
    Tokenizer* tz = tokenizer_from_str(s);
    if (tz->tkn->type == TKN_EOF) {
        return scamval_null();
    } else {
        scamval* ret = match_expr(tz);
        if (tz->tkn->type == TKN_EOF) {
            tokenizer_close(tz);
            return ret;
        } else {
            scamval_free(ret);
            return scamval_err("trailing input");
        }
    }
}

scamval* parse_file(char* fp) {
    Tokenizer* tz = tokenizer_from_file(fp);
    scamval* ret = match_expr_plus(tz);
    tokenizer_close(tz);
    return ret;
}

scamval* match_any_expr(Tokenizer* tz, int type, int start, int end) {
    if (tz->tkn->type == start) {
        TOKENIZER_MUST_ADVANCE(tz);
        scamval* ret = match_expr_star(tz);
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
        if (tz->tkn->type == TKN_LBRACE) {
            return match_any_expr(tz, SCAM_QUOTE, TKN_LBRACE, TKN_RBRACE);
        } else if (tz->tkn->type == TKN_LBRACKET) {
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

scamval* scamval_from_token(Token* tkn) {
    // this will need to be changed
    switch (tkn->type) {
        case TKN_INT: return scamval_int(strtoll(tkn->val, NULL, 10));
        case TKN_DEC: return scamval_dec(strtod(tkn->val, NULL));
        case TKN_SYM: 
            if (strcmp(tkn->val, "true") == 0) {
                return scamval_bool(1);
            } else if (strcmp(tkn->val, "false") == 0) {
                return scamval_bool(0);
            } else {
                return scamval_sym(tkn->val);
            }
        case TKN_STR:
            {
                // remove the quotes from string literals
                scamval* ret = scamval_str(tkn->val + 1);
                if (ret && ret->type == SCAM_STR) {
                    size_t n = strlen(ret->vals.s);
                    if (n > 0) {
                        ret->vals.s[n - 1] = '\0';
                    }
                }
                return ret;
            }
        default: return scamval_err("unknown token type");
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
