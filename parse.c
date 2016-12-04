#include "parse.h"
#include "tokenize.h"

// Forward declarations of recursive descent functions
scamval* match_expr(Tokenizer*);
scamval* match_expr_plus(Tokenizer*);
scamval* match_expr_star(Tokenizer*);
scamval* match_value_star(Tokenizer*);

scamval* parse_line(char* s) {
    Tokenizer* tz = tokenizer_from_str(s);
    scamval* ret = match_expr(tz);
    tokenizer_close(tz);
    return ret;
}

scamval* parse_file(char* fp) {
    Tokenizer* tz = tokenizer_from_file(fp);
    scamval* ret = match_expr(tz);
    tokenizer_close(tz);
    return ret;
}

scamval* match_expr(Tokenizer* tz) {
    return scamval_err("not implemented");
}

scamval* match_expr_plus(Tokenizer* tz) {
    return scamval_err("not implemented");
}

scamval* match_expr_star(Tokenizer* tz) {
    return scamval_err("not implemented");
}

scamval* match_value_star(Tokenizer* tz) {
    return scamval_err("not implemented");
}
