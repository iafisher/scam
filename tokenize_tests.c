#include <stdarg.h>
#include "tokenize.h"

void tkntest(char* s, int n, ...) {
    va_list args;
    va_start(args, n);
    Tokenizer tz; 
    tokenizer_from_str(&tz, s);
    int i = 0;
    while (i < n) {
        int this_type = va_arg(args, int);
        if (this_type != tz.tkn.type) {
            printf("Failed tokenize test \"%s\" on token %d; ", s, i);
            printf("got %s, expected %s\n", token_type_name(tz.tkn.type),
                                            token_type_name(this_type));
            return;
        }
        tokenizer_advance(&tz);
        i++;
    }
    va_end(args);
    tokenizer_close(&tz);
    printf("Passed tokenize test on \"%s\"\n", s);
}

void tokenize_tests() {
    tkntest("-2730", 1, TKN_INT);
    tkntest("valid-identifier", 1, TKN_SYM);
    tkntest("[8.9 -92.0]", 4, TKN_LBRACKET, TKN_DEC, TKN_DEC, TKN_RBRACKET);
    tkntest("\"this is a string literal\"", 1, TKN_STR);
    tkntest("a\"b\"c", 3, TKN_SYM, TKN_STR, TKN_SYM);
    tkntest("a{b}c", 5, TKN_SYM, TKN_LBRACE, TKN_SYM, TKN_RBRACE, TKN_SYM);
    tkntest("(+ 1 1)", 5, TKN_LPAREN, TKN_SYM, TKN_INT, TKN_INT, TKN_RPAREN);
    tkntest("", 1, TKN_EOF);
    tkntest("{+ - / *}", 6, TKN_LBRACE, TKN_SYM, TKN_SYM, TKN_SYM, TKN_SYM, TKN_RBRACE);
}
