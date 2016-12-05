#pragma once
#include <stdio.h>

#define FIRST_COL 0
#define FIRST_LINE 1

enum { STREAM_STR, STREAM_FILE };
typedef struct {
    int type; // STREAM_STR or STREAM_FILE
    int line, col;
    int mem_flag; // -1 if not remembering, > -1 otherwise
    int mem_len, mem_line, mem_col;
    // used by string streams
    int s_len;
    char* s;
    // used by file streams
    int last_pos;
    FILE* fp;
} Stream;

// Initialize streams from various sources
void stream_from_str(Stream*, const char*);
void stream_from_file(Stream*, const char*);

// Return 1 if the stream can still be read from
int stream_good(Stream*);

// Get the current character from the stream and advance the stream forward
char stream_getchar(Stream*);

// Turn memory mode on
void stream_mark(Stream*);

// Recall all characters since memory mode was turned on
// The return value begins with the last character seen before stream_mark was
// called, and ends with the character before the last call to stream_getchar
// The next call to stream_getchar will repeat the same character
char* stream_recall(Stream*);

// Free all stream resources, including the pointer itself
void stream_close(Stream*);
