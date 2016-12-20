#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "progutils.h"
#include "stream.h"

// Initialize a generic Stream object
void stream_init(Stream* strm, int type) {
    strm->type = type;
    strm->col = strm->mem_col = FIRST_COL;
    strm->line = strm->mem_line = FIRST_LINE;
    strm->mem_flag = -1;
    strm->chbuf = EOF;
    strm->good = 1;
    // initialize everything to 0 to placate valgrind
    strm->s_len = strm->last_pos = strm->mem_len = 0;
    strm->s = NULL;
    strm->fp = NULL;
}

void stream_from_str(Stream* strm, const char* s) {
    stream_init(strm, STREAM_STR);
    strm->s_len = strlen(s);
    strm->s = strdup(s);
}

void stream_from_file(Stream* strm, const char* fp) {
    stream_init(strm, STREAM_FILE);
    strm->fp = fopen(fp, "r");
}

char strstream_getchar(Stream* strm) {
    char ret = strm->s[strm->col++];
    if (strm->col > strm->s_len) {
        strm->good = 0;
        return EOF;
    } else {
        return ret;
    }
}

char fstream_getchar(Stream* strm) {
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

char stream_getchar(Stream* strm) {
    // return from the char buffer first, if it isn't empty
    if (strm->chbuf != EOF) {
        char ret = strm->chbuf;
        strm->chbuf = EOF;
        return ret;
    }
    if (!strm->good) {
        return EOF;
    }
    if (strm->mem_flag > -1) {
        strm->mem_len++;
    }
    switch (strm->type) {
        case STREAM_STR:  return strstream_getchar(strm);
        case STREAM_FILE: return fstream_getchar(strm);
        default: return EOF;
    }
}

void stream_retreat(Stream* strm) {
    switch (strm->type) {
        case STREAM_STR:
            if (strm->col > 0) {
                strm->col--;
                strm->good = 1;
            }
            break;
        case STREAM_FILE:
            fseek(strm->fp, strm->last_pos, SEEK_SET);
            break;
    }
}

void stream_mark(Stream* strm) {
    strm->mem_col = strm->col;
    strm->mem_line = strm->line;
    if (strm->type == STREAM_STR) {
        if (strm->col > FIRST_COL)
            strm->mem_flag = strm->col - 1;
    } else {
        strm->mem_flag = strm->last_pos;
    }
}

char* stream_recall(Stream* strm) {
    if (strm->mem_flag > -1) {
        char* ret = my_malloc(strm->mem_len + 1);
        if (strm->type == STREAM_STR) {
            strncpy(ret, strm->s + strm->mem_flag, strm->mem_len);
            ret[strm->mem_len] = '\0';
        } else {
            fseek(strm->fp, strm->mem_flag, SEEK_SET);
            fgets(ret, strm->mem_len + 1, strm->fp);
            // read and discard the current character
            fgetc(strm->fp);
        }
        strm->mem_flag = -1;
        strm->mem_len = 0;
        return ret;
    } else {
        return NULL;
    }
}

void stream_close(Stream* strm) {
    switch (strm->type) {
        case STREAM_STR:
            free(strm->s);
            break;
        case STREAM_FILE:
            fclose(strm->fp);
            break;
    }
}
