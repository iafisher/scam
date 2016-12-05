#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "tokenize.h"

// Initializer a Tokenizer object, regardless of the stream type
void tokenizer_init(Tokenizer* tz) {
    tz->tkn.val = NULL;
    tz->tkn.line = tz->tkn.col = 0;
}

void tokenizer_from_str(Tokenizer* tz, char* s) {
    tokenizer_init(tz);
    stream_from_str(&tz->strm, s);
    tokenizer_advance(tz);
}

void tokenizer_from_file(Tokenizer* tz, char* fp) {
    tokenizer_init(tz);
    stream_from_file(&tz->strm, fp);
    tokenizer_advance(tz);
}

int is_symbol_char(char c) {
    return isalnum(c) || strchr("-+?!%*/<>=_", c);
}

int is_symbol(char* s) {
    int n = strlen(s);
    if (n == 0) return 0;
    // first character cannot be a digit
    if (!is_symbol_char(s[0]) || isdigit(s[0])) return 0;
    for (int i = 1; i < n; i++) {
        if (!is_symbol_char(s[i]))
            return 0;
    }
    return 1;
}

int is_string_literal(char* s) {
    int n = strlen(s);
    return n >= 2 && s[0] == '"' && s[n - 1] == '"';
}

int is_integer(char* s) {
    int n = strlen(s);
    if (n == 0) return 0;
    if (s[0] == '-') return is_integer(s + 1);
    for (int i = 0; i < n; i++) {
        if (!isdigit(s[i]))
            return 0;
    }
    return 1;
}

int is_decimal(char* s) {
    int n = strlen(s);
    if (n == 0) return 0;
    if (s[0] == '-') return is_decimal(s + 1);
    int decimal_pos = -1;
    for (int i = 0; i < n; i++) {
        if (!isdigit(s[i])) {
            if (s[i] == '.') {
                if (decimal_pos == -1)
                    decimal_pos = i;
                else
                    // more than one decimal point
                    return 0;
            } else {
                // non-digit character
                return 0;
            }
        }
    }
    // make sure the decimal point isn't the last character
    return decimal_pos != n - 1;
}

// Return 1 if the character is a token
int is_token(char c) {
    return strchr("(){}[]", c) != NULL;
}

// Return 1 if the character is a token boundary
int is_token_boundary(char c) {
    return is_token(c)  || isspace(c) || c == '"' || c == '\0';
}

// Return the token type of the given string
int token_type_from_str(char* s) {
    // make sure is_integer comes before is_symbol, or else "-1" is a symbol
    if (is_integer(s)) {
        return TKN_INT;
    } else if (is_decimal(s)) {
        return TKN_DEC;
    } else if (is_string_literal(s)) {
        return TKN_STR;
    } else if (is_symbol(s)) {
        return TKN_SYM;
    } else if (strcmp(s, "(") == 0) {
        return TKN_LPAREN;
    } else if (strcmp(s, ")") == 0) {
        return TKN_RPAREN;
    } else if (strcmp(s, "[") == 0) {
        return TKN_LBRACKET;
    } else if (strcmp(s, "]") == 0) {
        return TKN_RBRACKET;
    } else if (strcmp(s, "{") == 0) {
        return TKN_LBRACE;
    } else if (strcmp(s, "}") == 0) {
        return TKN_RBRACE;
    } else {
        return TKN_UNKNOWN;
    }
}

// Set the tkn member from the stream memory
void set_token_from_stream_memory(Tokenizer* tz) {
    tz->tkn.line = tz->strm.mem_line;
    tz->tkn.col = tz->strm.mem_col - 1;
    if (tz->tkn.val) free(tz->tkn.val);
    tz->tkn.val = stream_recall(&tz->strm);
    tz->tkn.type = token_type_from_str(tz->tkn.val);
}

// Set the EOF token
void set_eof_token(Tokenizer* tz) {
    tz->tkn.line = tz->strm.line;
    tz->tkn.col = tz->strm.col;
    tz->tkn.type = TKN_EOF;
}

void tokenizer_advance(Tokenizer* tz) {
    // skip whitespace
    char c = ' ';
    while (stream_good(&tz->strm) && isspace(c))
        c = stream_getchar(&tz->strm);
    if (!stream_good(&tz->strm)) {
        set_eof_token(tz);
    } else if (c == ';') {
        // skip comments
        while (stream_good(&tz->strm) && c != '\n')
            c = stream_getchar(&tz->strm);
        tokenizer_advance(tz);
    } else if (is_token(c)) {
        stream_mark(&tz->strm);
        stream_getchar(&tz->strm);
        set_token_from_stream_memory(tz);
    } else if (c == '"') {
        stream_mark(&tz->strm);
        // find the end of the string literal
        // this should eventually be adjusted to account for backslash escapes
        c = stream_getchar(&tz->strm);
        while (c != '"')
            c = stream_getchar(&tz->strm);
        stream_getchar(&tz->strm);
        set_token_from_stream_memory(tz);
    } else {
        stream_mark(&tz->strm);
        // find the end of the token
        while (!is_token_boundary(c))
            c = stream_getchar(&tz->strm);
        set_token_from_stream_memory(tz);
    }
}

void tokenizer_close(Tokenizer* tz) {
    stream_close(&tz->strm);
    if (tz->tkn.val) {
        free(tz->tkn.val);
    }
}

const char* token_type_name(int type) {
    switch (type) {
        case TKN_RPAREN: return "TKN_RPAREN";
        case TKN_LPAREN: return "TKN_LPAREN";
        case TKN_RBRACKET: return "TKN_RBRACKET";
        case TKN_LBRACKET: return "TKN_LBRACKET";
        case TKN_RBRACE: return "TKN_RBRACE";
        case TKN_LBRACE: return "TKN_LBRACE";
        case TKN_INT: return "TKN_INT";
        case TKN_DEC: return "TKN_DEC";
        case TKN_SYM: return "TKN_SYM";
        case TKN_STR: return "TKN_STR";
        case TKN_EOF: return "TKN_EOF";
        case TKN_UNKNOWN: return "TKN_UNKNOWN";
        default: return "bad token type";
    }
}

void print_all_tokens(Tokenizer* tz) {
    while (tz->tkn.type != TKN_EOF) {
        printf("%s (%s)", tz->tkn.val, token_type_name(tz->tkn.type));
        printf(" at line %d, col %d\n", tz->tkn.line, tz->tkn.col);
        tokenizer_advance(tz);
    }
}
