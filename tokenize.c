#include <stdlib.h>
#include <string.h>
#include "tokenize.h"

Tokenizer* tokenizer_from_str(char* s) {
    Tokenizer* ret = malloc(sizeof(Tokenizer));
    if (ret) {
        ret->strm = stream_from_str(s);
        tokenizer_advance(ret);
    }
    return ret;
}

Tokenizer* tokenizer_from_file(char* fp) {
    Tokenizer* ret = malloc(sizeof(Tokenizer));
    if (ret) {
        ret->strm = stream_from_file(fp);
        tokenizer_advance(ret);
    }
    return ret;
}

void tokenizer_advance(Tokenizer* tz) {
    // to be implemented
}

void tokenizer_close(Tokenizer* tz) {
    if (tz) {
        stream_close(tz->strm);
        free(tz);
    }
}

void print_all_tokens(Tokenizer* tz) {
    // to be implemented
}
