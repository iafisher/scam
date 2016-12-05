#include <stdarg.h>
#include "tokenize.h"

void runtest(char* s, int n, ...) {
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
    runtest("-2730", 1, TKN_INT);
    runtest("valid-identifier", 1, TKN_SYM);
    runtest("[8.9 -92.0]", 4, TKN_LBRACKET, TKN_DEC, TKN_DEC, TKN_RBRACKET);
    runtest("\"this is a string literal\"", 1, TKN_STR);
    runtest("a\"b\"c", 3, TKN_SYM, TKN_STR, TKN_SYM);
    runtest("a{b}c", 5, TKN_SYM, TKN_LBRACE, TKN_SYM, TKN_RBRACE, TKN_SYM);
    runtest("(+ 1 1)", 5, TKN_LPAREN, TKN_SYM, TKN_INT, TKN_INT, TKN_RPAREN);
    runtest("", 1, TKN_EOF);
    runtest("{+ - / *}", 6, TKN_LBRACE, TKN_SYM, TKN_SYM, TKN_SYM, TKN_SYM, TKN_RBRACE);
}
