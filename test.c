#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "eval.h"
#include "parse.h"
#include "stream.h"
#include "tokenize.h"

#define CHECK_OBJ(obj, name) \
    { if (!obj) { printf("%s not yet opened\n", name); return; } }

void stream_repl(char*, size_t, Stream**);
void tokenize_repl(char*, size_t);
void parse_repl(char*);
void eval_repl(char*, scamenv*);

void stream_tests();

enum { REPL_EVAL, REPL_PARSE, REPL_TOKENIZE, REPL_STREAM };
int main(int argc, char** argv) {
    // Run tests
    //stream_tests();
    // set mode based on the command line argument
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
    // Run the REPL
    char* buffer = NULL;
    size_t s_len = 0;
    Stream* strm = NULL;
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    while (1) {
        printf(">>> ");
        int end = getline(&buffer, &s_len, stdin);
        // remove trailing newline
        if (end > 0) {
            buffer[end - 1] = '\0';
        }
        if (strcmp(buffer, "quit") == 0) {
            break;
        } else if (strcmp(buffer, "!eval") == 0) {
            mode = REPL_EVAL;
            continue;
        } else if (strcmp(buffer, "!parse") == 0) {
            mode = REPL_PARSE;
            continue;
        } else if (strcmp(buffer, "!tokenize") == 0) {
            mode = REPL_TOKENIZE;
            continue;
        } else if (strcmp(buffer, "!stream") == 0) {
            mode = REPL_STREAM;
            continue;
        }
        if (mode == REPL_STREAM) {
            stream_repl(buffer, s_len, &strm);
        } else if (mode == REPL_TOKENIZE) {
            tokenize_repl(buffer, s_len);
        } else if (mode == REPL_PARSE) {
            parse_repl(buffer);
        } else if (mode == REPL_EVAL) {
            eval_repl(buffer, env);
        }
    }
    if (buffer) free(buffer);
    stream_close(strm);
    scamenv_free(env);
    return 0;
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
        if (strcmp(command, "getchar") == 0) {
            printf("'%c'\n", stream_getchar(*strm));
        } else if (strcmp(command, "mark") == 0) {
            stream_mark(*strm);
            printf("Set mem_flag to %d\n", (*strm)->mem_flag);
        } else if (strcmp(command, "recall") == 0) {
            char* s = stream_recall(*strm);
            printf("\"%s\"\n", s);
            free(s);
        } else {
            printf("unknown command\n");
        }
    }
}

void tokenize_repl(char* command, size_t s_len) {
    if (strstr(command, "open") == command && s_len >= 6) {
        Tokenizer* tz = tokenizer_from_file(command + 5);
        if (tz) {
            print_all_tokens(tz);
            tokenizer_close(tz);
        } else {
            printf("Error: unable to open tokenizer from file");
        }
    } else {
        Tokenizer* tz = tokenizer_from_str(command);
        if (tz) {
            print_all_tokens(tz);
            tokenizer_close(tz);
        } else {
            printf("Error: unable to open tokenizer");
        }
    }
}

void parse_repl(char* command) {
    scamval* ast = parse_line(command);
    scamval_println(ast);
    scamval_free(ast);
}

void eval_repl(char* command, scamenv* env) {
    scamval* v = eval_line(command, env);
    scamval_println(v);
    scamval_free(v);
}

void stream_test_103_27(Stream* strm) {
    assert(stream_good(strm));
    // token: (
    assert(stream_getchar(strm) == '(');
    // token: +
    assert(stream_getchar(strm) == '+');
    stream_mark(strm);
    assert(stream_getchar(strm) == ' ');
    char* s = stream_recall(strm);
    assert(strcmp(s, "+") == 0);
    free(s);
    // token: 103
    assert(stream_getchar(strm) == '1');
    stream_mark(strm);
    assert(stream_getchar(strm) == '0');
    assert(stream_getchar(strm) == '3');
    assert(stream_getchar(strm) == ' ');
    s = stream_recall(strm);
    assert(strcmp(s, "103") == 0);
    free(s);
    // token: 27
    assert(stream_getchar(strm) == '2');
    stream_mark(strm);
    assert(stream_getchar(strm) == '7');
    assert(stream_getchar(strm) == ')');
    s = stream_recall(strm);
    assert(strcmp(s, "27") == 0);
    free(s);
    // token: )
    assert(stream_getchar(strm) == ')');
}

void stream_tests() {
    // open the two streams
    Stream* fstream = stream_from_file("streamtest.txt");
    Stream* sstream = stream_from_str("(+ 103 27)");
    stream_test_103_27(fstream);
    stream_test_103_27(sstream);
    // close the two streams
    stream_close(fstream); 
    stream_close(sstream);
}
