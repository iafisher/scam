#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"

// Initialize a generic Stream object
Stream* stream_init(int type) {
    Stream* ret = malloc(sizeof(Stream));
    if (ret) {
        ret->type = type;
        ret->col = ret->mem_col = 0;
        ret->line = ret->mem_line = 1;
        ret->mem_flag = -1;
        // initialize all type-dependent members to 0
        ret->s = NULL;
        ret->fp = NULL;
        ret->s_len = ret->last_pos = ret->mem_len = 0;
    }
    return ret;
}

Stream* stream_from_str(const char* s) {
    Stream* ret = stream_init(STREAM_STR);
    if (ret) {
        ret->s_len = strlen(s);
        ret->s = malloc(ret->s_len + 1);
        if (ret->s) {
            strcpy(ret->s, s);
        } else {
            ret->s_len = 0;
        }
    }
    return ret;
}

Stream* stream_from_file(const char* fp) {
    Stream* ret = stream_init(STREAM_FILE);
    if (ret) {
        ret->last_pos = 0;
        ret->mem_len = 0;
        ret->fp = fopen(fp, "r");
    }
    return ret;
}

int stream_good(Stream* strm) {
    if (strm->type == STREAM_STR) {
        return strm->col < strm->s_len;
    } else {
        return strm->fp && !ferror(strm->fp) && !feof(strm->fp);
    }
}

char stream_getchar(Stream* strm) {
    if (!stream_good(strm)) return '\0';
    if (strm->mem_flag > -1) 
        strm->mem_len++;
    if (strm->type == STREAM_STR) {
        return strm->s[strm->col++];
    } else {
        strm->last_pos = ftell(strm->fp);
        char c = fgetc(strm->fp);
        if (c == '\n') {
            strm->col = 0;
            strm->line++;
        } else {
            strm->col++;
        }
        return c;
    }
}

void stream_putchar(Stream* strm) {
    if (strm->mem_flag == -1) {
        if (strm->type == STREAM_STR) {
            if (strm->col > 0) {
                strm->col--;
            }
        } else {
            fseek(strm->fp, strm->last_pos, SEEK_SET);
        }
    }
}

void stream_mark(Stream* strm) {
    strm->mem_col = strm->col;
    strm->mem_line = strm->line;
    if (strm->type == STREAM_STR) {
        if (strm->col > 0)
            strm->mem_flag = strm->col - 1;
    } else {
        strm->mem_flag = strm->last_pos;
    }
}

char* stream_recall(Stream* strm) {
    if (strm->mem_flag > -1) {
        char* ret = malloc(strm->mem_len + 1);
        if (ret) {
            if (strm->type == STREAM_STR) {
                strncpy(ret, strm->s + strm->mem_flag, strm->mem_len);
                ret[strm->mem_len] = '\0';
            } else {
                fseek(strm->fp, strm->mem_flag, SEEK_SET);
                fgets(ret, strm->mem_len + 1, strm->fp);
                // read and discard the current character
                fgetc(strm->fp);
            }
        }
        strm->mem_flag = -1;
        strm->mem_len = 0;
        return ret;
    } else {
        return NULL;
    }
}

void stream_close(Stream* strm) {
    if (!strm) return;
    if (strm->type == STREAM_STR) {
        if (strm->s) {
            free(strm->s);
        }
    } else {
        if (strm->fp) {
            fclose(strm->fp);
        }
    }
    free(strm);
}
