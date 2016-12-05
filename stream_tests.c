#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"
#include "tests.h"

int stream_test_103_27(Stream* strm) {
    MY_ASSERT(stream_good(strm));
    // token: (
    MY_ASSERT(stream_getchar(strm) == '(');
    // token: +
    MY_ASSERT(stream_getchar(strm) == '+');
    stream_mark(strm);
    MY_ASSERT(stream_getchar(strm) == ' ');
    char* s = stream_recall(strm);
    MY_ASSERT(strcmp(s, "+") == 0);
    free(s);
    // token: 103
    MY_ASSERT(stream_getchar(strm) == ' '); // getchar repeats itself after recall
    MY_ASSERT(stream_getchar(strm) == '1');
    stream_mark(strm);
    MY_ASSERT(stream_getchar(strm) == '0');
    MY_ASSERT(stream_getchar(strm) == '3');
    MY_ASSERT(stream_getchar(strm) == ' ');
    s = stream_recall(strm);
    MY_ASSERT(strcmp(s, "103") == 0);
    free(s);
    // token: 27
    MY_ASSERT(stream_getchar(strm) == ' ');
    MY_ASSERT(stream_getchar(strm) == '2');
    stream_mark(strm);
    MY_ASSERT(stream_getchar(strm) == '7');
    MY_ASSERT(stream_getchar(strm) == ')');
    s = stream_recall(strm);
    MY_ASSERT(strcmp(s, "27") == 0);
    free(s);
    // token: )
    MY_ASSERT(stream_getchar(strm) == ')');
    return 1;
}

void stream_tests() {
    // open the two streams
    Stream fstream, sstream; 
    stream_from_file(&fstream, "streamtest.txt");
    stream_from_str(&sstream, "(+ 103 27)");
    printf("Running file stream test... ");
    if (stream_test_103_27(&fstream)) {
        printf("passed!\n");
    } else {
        printf("FAILED (oh no)\n");
    }
    printf("Running string stream test... ");
    if (stream_test_103_27(&sstream)) {
        printf("passed!\n");
    } else {
        printf("FAILED (oh no)\n");
    }
    // close the two streams
    stream_close(&fstream); 
    stream_close(&sstream);
}
