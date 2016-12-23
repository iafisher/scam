#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "stream.h"

// Initialize a generic Stream object
void stream_init(Stream* strm) {
    strm->col = strm->mem_col = FIRST_COL;
    strm->line = strm->mem_line = FIRST_LINE;
    strm->mem_flag = -1;
    strm->good = 1;
    // initialize everything to 0 to placate valgrind
    strm->last_pos = strm->mem_len = 0;
    strm->s = NULL;
}

void stream_from_str(Stream* strm, const char* s) {
    stream_init(strm);
    strm->s = strdup(s);
    strm->fp = fmemopen(strm->s, strlen(s) + 1, "r");
}

void stream_from_file(Stream* strm, const char* fp) {
    stream_init(strm);
    strm->fp = fopen(fp, "r");
}

char stream_getc(Stream* strm) {
    if (!strm->good) {
        return EOF;
    }
    if (strm->mem_flag > -1) {
        strm->mem_len++;
    }
    strm->last_pos = ftell(strm->fp);
    char c = fgetc(strm->fp);
    if (c == EOF) {
        strm->good = 0;
    } else if (c == '\n') {
        strm->col = FIRST_COL;
        strm->line++;
    } else {
        strm->col++;
    }
    return c;
}

void stream_ungetc(Stream* strm, char c) {
    ungetc(c, strm->fp);
    if (strm->col > 0)
        strm->col--;
}

void stream_mark(Stream* strm) {
    strm->mem_col = strm->col;
    strm->mem_line = strm->line;
    strm->mem_flag = strm->last_pos;
}

char* stream_recall(Stream* strm) {
    if (strm->mem_flag > -1) {
        char* ret = gc_malloc(strm->mem_len + 1);
        fseek(strm->fp, strm->mem_flag, SEEK_SET);
        fgets(ret, strm->mem_len + 1, strm->fp);
        // read and discard the current character
        fgetc(strm->fp);
        strm->mem_flag = -1;
        strm->mem_len = 0;
        return ret;
    } else {
        return NULL;
    }
}

void stream_close(Stream* strm) {
    fclose(strm->fp);
    free(strm->s);
}
