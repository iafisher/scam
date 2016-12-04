#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"

#define CHECK_OBJ(obj, name) \
    { if (!obj) { printf("%s not yet opened\n", name); return; } }

void stream_repl(char*, size_t, Stream**);

enum { REPL_EVAL, REPL_PARSE, REPL_TOKENIZE, REPL_STREAM };
int main(int argc, char** argv) {
    char* buffer = NULL;
    size_t s_len = 0;
    int mode = REPL_EVAL;
    if (argc == 2) {
        if (strcmp(argv[1], "stream") == 0) {
            mode = REPL_STREAM;
        } else if (strcmp(argv[1], "tokenize") == 0) {
            mode = REPL_TOKENIZE;
        } else if (strcmp(argv[1], "parse") == 0) {
            mode = REPL_PARSE;
        }
    }
    Stream* strm = NULL;
    while (1) {
        printf(">>> ");
        getline(&buffer, &s_len, stdin);
        if (strcmp(buffer, "quit\n") == 0) {
            break;
        } else if (strcmp(buffer, "!eval\n") == 0) {
            mode = REPL_EVAL;
            continue;
        } else if (strcmp(buffer, "!parse\n") == 0) {
            mode = REPL_PARSE;
            continue;
        } else if (strcmp(buffer, "!tokenize\n") == 0) {
            mode = REPL_TOKENIZE;
            continue;
        } else if (strcmp(buffer, "!stream\n") == 0) {
            mode = REPL_STREAM;
            continue;
        }
        if (mode == REPL_STREAM) {
            stream_repl(buffer, s_len, &strm);
        }
    }
    if (buffer) free(buffer);
    stream_close(strm);
}

void stream_repl(char* command, size_t s_len, Stream** strm) {
    if (strstr(command, "open") == command && s_len >= 6) {
        char* fp = command + 5;
        stream_close(*strm);
        *strm = stream_from_file(fp);
    } else if (strstr(command, "feed") == command && s_len >= 6) {
        char* line = command + 5;
        stream_close(*strm);
        *strm = stream_from_str(line);
    } else {
        CHECK_OBJ(strm, "stream");
        if (strcmp(command, "getch\n") == 0) {
            printf("'%c'\n", stream_getchar(*strm));
        } else if (strcmp(command, "mark\n") == 0) {
            stream_mark(*strm);
        } else if (strcmp(command, "recall\n") == 0) {
            char* s = stream_recall(*strm);
            printf("\"%s\"\n", s);
            free(s);
        } else if (strcmp(command, "memory_flag\n") == 0) {
            printf("%d\n", (*strm)->memory_flag);
        } else if (strcmp(command, "memory_len\n") == 0) {
            printf("%d\n", (*strm)->memory_len);
        } else {
            printf("unknown command\n");
        }
    }
}
