#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"
#include "tests.h"

#define GETCHAR_ASSERT(strm, c) { \
    int got = stream_getchar(strm); \
    if (got != c) { \
        fprintf(stderr, "failure at %s:%d ", __FILE__, __LINE__); \
        fprintf(stderr, "(%s)", strm->type == STREAM_STR ? "string":"file"); \
        fprintf(stderr, " expected %c (%d), ", c, c); \
        fprintf(stderr, "got %c (%d)\n", got, got); \
        exit(EXIT_FAILURE); \
    } \
}

#define RECALL_ASSERT(strm, s) { \
    char* got = stream_recall(strm); \
    if (strcmp(got, s) != 0) { \
        fprintf(stderr, "failure at %s:%d ", __FILE__, __LINE__); \
        fprintf(stderr, "(%s)", strm->type == STREAM_STR ? "string":"file"); \
        fprintf(stderr, " expected \"%s\", got \"%s\"\n", s, got); \
        exit(EXIT_FAILURE); \
    } \
    free(got); \
}

typedef void (*streamtest_f)(Stream*);

void run_stream_test(streamtest_f, char* line, char* fp);
void stream_test_103_27(Stream* strm);

void stream_tests() {
    run_stream_test(stream_test_103_27, "(+ 103 27)", 
                                        "test_files/streamtest.txt");
}

void run_stream_test(streamtest_f func, char* line, char* fp) {
    Stream fstream, sstream; 
    stream_from_file(&fstream, fp);
    stream_from_str(&sstream, line);
    //func(&fstream);
    func(&sstream);
    stream_close(&fstream); 
    stream_close(&sstream);
}

void stream_test_103_27(Stream* strm) {
    // token: (
    GETCHAR_ASSERT(strm, '(');
    stream_mark(strm);
    GETCHAR_ASSERT(strm, '+');
    RECALL_ASSERT(strm, "(");
    stream_retreat(strm);
    // token: +
    GETCHAR_ASSERT(strm, '+');
    stream_mark(strm);
    GETCHAR_ASSERT(strm, ' ');
    RECALL_ASSERT(strm, "+");
    stream_retreat(strm);
    // token: 103
    GETCHAR_ASSERT(strm, ' ');
    GETCHAR_ASSERT(strm, '1');
    stream_mark(strm);
    GETCHAR_ASSERT(strm, '0');
    GETCHAR_ASSERT(strm, '3');
    GETCHAR_ASSERT(strm, ' ');
    RECALL_ASSERT(strm, "103");
    stream_retreat(strm);
    // token: 27
    GETCHAR_ASSERT(strm, ' ');
    GETCHAR_ASSERT(strm, '2');
    stream_mark(strm);
    GETCHAR_ASSERT(strm, '7');
    GETCHAR_ASSERT(strm, ')');
    RECALL_ASSERT(strm, "27");
    stream_retreat(strm);
    // token: )
    GETCHAR_ASSERT(strm, ')');
    stream_mark(strm);
    stream_getchar(strm);
    RECALL_ASSERT(strm, ")");
    GETCHAR_ASSERT(strm, EOF);
}
