#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"

// Initialize a generic Stream object
Stream* stream_init(int type) {
    Stream* ret = malloc(sizeof(Stream));
    if (ret) {
        ret->type = type;
        ret->col = 0;
        ret->line = 1;
        ret->memory_flag = -1;
        // initialize all type-dependent members to 0
        ret->s = NULL;
        ret->fp = NULL;
        ret->s_len = ret->last_pos = ret->memory_len = 0;
    }
    return ret;
}

// Initialize a Stream object from a string
Stream* stream_from_str(char* s) {
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

// Initialize a Stream object from a file path
Stream* stream_from_file(char* fp) {
    Stream* ret = stream_init(STREAM_FILE);
    if (ret) {
        ret->last_pos = 0;
        ret->memory_len = 0;
        ret->fp = fopen(fp, "r");
    }
    return ret;
}

// Check if a Stream object can be read from
int stream_good(Stream* strm) {
    if (strm->type == STREAM_STR) {
        return strm->col < strm->s_len;
    } else {
        return strm->fp && !ferror(strm->fp) && !feof(strm->fp);
    }
}

// Read a character from the Stream object
char stream_getchar(Stream* strm) {
    if (!stream_good(strm)) return '\0';
    if (strm->memory_flag > -1) 
        strm->memory_len++;
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

// Tell the Stream object to start remembering characters
void stream_mark(Stream* strm) {
    if (strm->type == STREAM_STR) {
        if (strm->col > 0)
            strm->memory_flag = strm->col - 1;
    } else {
        strm->memory_flag = strm->last_pos;
    }
}

// Recall the characters remembered so far
char* stream_recall(Stream* strm) {
    if (strm->memory_flag > -1) {
        char* ret = malloc(strm->memory_len + 1);
        if (ret) {
            if (strm->type == STREAM_STR) {
                strncpy(ret, strm->s + strm->memory_flag, strm->memory_len);
                ret[strm->memory_len] = '\0';
            } else {
                fseek(strm->fp, strm->memory_flag, SEEK_SET);
                fgets(ret, strm->memory_len + 1, strm->fp);
                // read and discard the current character
                fgetc(strm->fp);
            }
        }
        strm->memory_flag = -1;
        strm->memory_len = 0;
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
