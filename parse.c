#include <stdlib.h>
#include <string.h>
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
scamval* match_expr(Tokenizer*);
scamval* match_expr_star(Tokenizer*);
scamval* match_expr_plus(Tokenizer*);
scamval* match_value(Tokenizer*);
int starts_expr(int tkn_type);
int starts_value(int tkn_type);
scamval* scamval_from_token(Token*);

scamval* parse_generic(char* s, tokenizer_init_t tz_init, match_t match_f) {
    Tokenizer tz; 
    tz_init(&tz, s);
    if (tz.tkn.type == TKN_EOF) {
        tokenizer_close(&tz);
        return scamnull();
    } else {
        scamval* ret = match_f(&tz);
        if (tz.tkn.type == TKN_EOF) {
            tokenizer_close(&tz);
            return ret;
        } else {
            scamval_free(ret);
            tokenizer_close(&tz);
            return scamerr("trailing input");
        }
    }
}

scamval* parse_line(char* s) {
    return parse_generic(s, tokenizer_from_str, match_expr);
}

scamval* parse_file(char* fp) {
    return parse_generic(fp, tokenizer_from_file, match_program);
}

scamval* match_program(Tokenizer* tz) {
    scamval* ast = match_expr_plus(tz);
    if (ast->type != SCAM_ERR) {
        scamval_prepend(ast, scamsym("begin"));
        return ast;
    } else {
        return ast;
    }
}

scamval* match_any_expr(Tokenizer* tz, int type, int start, int end) {
    if (tz->tkn.type == start) {
        int line = tokenizer_line(tz);
        int col = tokenizer_col(tz);
        TOKENIZER_MUST_ADVANCE(tz);
        scamval* ret = match_expr_star(tz);
        ret->type = type;
        if (tz->tkn.type == end) {
            tokenizer_advance(tz);
            return ret;
        } else {
            scamval_free(ret);
            return scamerr("expected end of expression started at line %d, "
                           "col %d", line, col);
        }
    } else {
        return scamerr("expected start of expression");
    }
}

scamval* match_any_nonempty_expr(Tokenizer* tz, int type, int start, int end) {
    scamval* ret = match_any_expr(tz, type, start, end);
    if (ret->vals.arr->count > 0) {
        return ret;
    } else {
        scamval_free(ret);
        return scamerr("empty expression");
    }
}

scamval* match_expr(Tokenizer* tz) {
    if (tz->tkn.type == TKN_LPAREN) {
        return match_any_nonempty_expr(tz, SCAM_CODE, TKN_LPAREN, TKN_RPAREN);
    } else {
        return match_value(tz);
    }
}

scamval* match_expr_plus(Tokenizer* tz) {
    scamval* ast = scamcode();
    scamval* first_expr = match_expr(tz);
    if (first_expr->type != SCAM_ERR) {
        scamval_append(ast, first_expr);
    } else {
        scamval_free(ast);
        return first_expr;
    }
    while (starts_expr(tz->tkn.type))
        scamval_append(ast, match_expr(tz));
    return ast;
}

scamval* match_expr_star(Tokenizer* tz) {
    scamval* ast = scamcode();
    while (starts_expr(tz->tkn.type))
        scamval_append(ast, match_expr(tz));
    return ast;
}

scamval* match_value(Tokenizer* tz) {
    if (starts_value(tz->tkn.type)) {
        if (tz->tkn.type == TKN_LBRACE) {
            return match_any_expr(tz, SCAM_QUOTE, TKN_LBRACE, TKN_RBRACE);
        } else if (tz->tkn.type == TKN_LBRACKET) {
            return match_any_expr(tz, SCAM_LIST, TKN_LBRACKET, TKN_RBRACKET);
        } else {
            scamval* ret = scamval_from_token(&tz->tkn);
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
                if (ret && ret->type == SCAM_STR) {
                    size_t n = strlen(ret->vals.s);
                    if (n > 0) {
                        ret->vals.s[n - 1] = '\0';
                    }
                }
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
