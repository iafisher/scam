#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"
#include "tests.h"

#define MY_ASSERT(cond) if (!(cond)) return 0;

int stream_test_103_27(Stream* strm) {
    TEST_ASSERT(stream_good(strm));
    // token: (
    TEST_ASSERT(stream_getchar(strm) == '(');
    stream_mark(strm);
    TEST_ASSERT(stream_getchar(strm) == '+');
    char* s = stream_recall(strm);
    TEST_ASSERT(strcmp(s, "(") == 0);
    free(s);
    stream_putchar(strm, '+');
    // token: +
    TEST_ASSERT(stream_getchar(strm) == '+');
    stream_mark(strm);
    TEST_ASSERT(stream_getchar(strm) == ' ');
    s = stream_recall(strm);
    TEST_ASSERT(strcmp(s, "+") == 0);
    free(s);
    stream_putchar(strm, ' ');
    // token: 103
    TEST_ASSERT(stream_getchar(strm) == ' ');
    TEST_ASSERT(stream_getchar(strm) == '1');
    stream_mark(strm);
    TEST_ASSERT(stream_getchar(strm) == '0');
    TEST_ASSERT(stream_getchar(strm) == '3');
    TEST_ASSERT(stream_getchar(strm) == ' ');
    s = stream_recall(strm);
    TEST_ASSERT(strcmp(s, "103") == 0);
    free(s);
    stream_putchar(strm, ' ');
    // token: 27
    TEST_ASSERT(stream_getchar(strm) == ' ');
    TEST_ASSERT(stream_getchar(strm) == '2');
    stream_mark(strm);
    TEST_ASSERT(stream_getchar(strm) == '7');
    TEST_ASSERT(stream_getchar(strm) == ')');
    s = stream_recall(strm);
    TEST_ASSERT(strcmp(s, "27") == 0);
    free(s);
    stream_putchar(strm, ')');
    // token: )
    TEST_ASSERT(stream_getchar(strm) == ')');
    stream_mark(strm);
    stream_getchar(strm);
    s = stream_recall(strm);
    TEST_ASSERT(strcmp(s, ")") == 0);
    free(s);
    return 1;
}

void stream_tests() {
    // open the two streams
    Stream fstream, sstream; 
    stream_from_file(&fstream, "test_files/streamtest.txt");
    stream_from_str(&sstream, "(+ 103 27)");
    stream_test_103_27(&fstream);
    stream_test_103_27(&sstream);
    // close the two streams
    stream_close(&fstream); 
    stream_close(&sstream);
}
