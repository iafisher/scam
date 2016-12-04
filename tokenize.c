#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "tokenize.h"

Tokenizer* tokenizer_from_str(char* s) {
    Tokenizer* ret = malloc(sizeof(Tokenizer));
    if (ret) {
        ret->strm = stream_from_str(s);
        ret->tkn = malloc(sizeof(Token));
        if (ret->tkn) ret->tkn->val = NULL;
        tokenizer_advance(ret);
    }
    return ret;
}

Tokenizer* tokenizer_from_file(char* fp) {
    Tokenizer* ret = malloc(sizeof(Tokenizer));
    if (ret) {
        ret->strm = stream_from_file(fp);
        ret->tkn = malloc(sizeof(Token));
        if (ret->tkn) ret->tkn->val = NULL;
        tokenizer_advance(ret);
    }
    return ret;
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
    return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']';
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
    } else {
        return TKN_UNKNOWN;
    }
}

// Set the tkn member from the stream memory
void set_token_from_stream_memory(Tokenizer* tz) {
    tz->tkn->line = tz->strm->mem_line;
    tz->tkn->col = tz->strm->mem_col - 1;
    if (tz->tkn->val) free(tz->tkn->val);
    tz->tkn->val = stream_recall(tz->strm);
    tz->tkn->type = token_type_from_str(tz->tkn->val);
}

// Set the tkn member based on the given character
void set_token_from_char(Tokenizer* tz, char c) {
    tz->tkn->line = tz->strm->line;
    tz->tkn->col = tz->strm->col - 1;
    if (tz->tkn->val) free(tz->tkn->val);
    tz->tkn->val = malloc(2);
    if (tz->tkn->val) {
        tz->tkn->val[0] = c;
        tz->tkn->val[1] = '\0';
    }
    switch (c) {
        case '(': tz->tkn->type = TKN_LPAREN; break;
        case ')': tz->tkn->type = TKN_RPAREN; break;
        case '{': tz->tkn->type = TKN_LBRACE; break;
        case '}': tz->tkn->type = TKN_RBRACE; break;
        case '[': tz->tkn->type = TKN_LBRACKET; break;
        case ']': tz->tkn->type = TKN_RBRACKET; break;
        default: tz->tkn->type = TKN_UNKNOWN;
    }
}

// Set the EOF token
void set_eof_token(Tokenizer* tz) {
    tz->tkn->line = tz->strm->line;
    tz->tkn->col = tz->strm->col;
    tz->tkn->type = TKN_EOF;
}

void tokenizer_advance(Tokenizer* tz) {
    // skip whitespace
    char c = ' ';
    while (stream_good(tz->strm) && isspace(c))
        c = stream_getchar(tz->strm);
    if (!stream_good(tz->strm)) {
        set_eof_token(tz);
    } else if (is_token(c)) {
        set_token_from_char(tz, c);
    } else if (c == '"') {
        stream_mark(tz->strm);
        // find the end of the string literal
        // this should eventually be adjusted to account for backslash escapes
        c = stream_getchar(tz->strm);
        while (c != '"')
            c = stream_getchar(tz->strm);
        c = stream_getchar(tz->strm);
        set_token_from_stream_memory(tz);
        // put the character back on the stream
        if (!isspace(c)) {
            stream_putchar(tz->strm);
        }
    } else {
        stream_mark(tz->strm);
        // find the end of the token
        while (!isspace(c) && !is_token(c))
            c = stream_getchar(tz->strm);
        set_token_from_stream_memory(tz);
        // put the token back on the stream
        if (is_token(c)) {
            stream_putchar(tz->strm);
        }
    }
}

void tokenizer_close(Tokenizer* tz) {
    if (tz) {
        stream_close(tz->strm);
        if (tz->tkn) {
            if (tz->tkn->val)
                free(tz->tkn->val);
            free(tz->tkn);
        }
        free(tz);
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
    while (tz->tkn->type != TKN_EOF) {
        printf("%s (%s)", tz->tkn->val, token_type_name(tz->tkn->type));
        printf(" at line %d, col %d\n", tz->tkn->line, tz->tkn->col);
        tokenizer_advance(tz);
    }
}
