#pragma once
#include <stdio.h>

#define FIRST_COL 0
#define FIRST_LINE 1

typedef struct {
    int line, col;
    int good;
    int mem_flag; // -1 if not remembering, ftell pos when marked otherwise
    int mem_len; // length of characters remembered so far
    int mem_line, mem_col; // line and col when memory mode began
    int last_pos; // ftell pos before last call to stream_getc
    FILE* fp;
    char* s; // copy of string, if initialized from string
} Stream;

// Initialize streams from various sources
void stream_from_str(Stream*, const char*);
void stream_from_file(Stream*, const char*);

// Get the current character from the stream and advance the stream forward
char stream_getc(Stream*);
// Unget the char back onto the stream (don't do this while in memory mode!)
void stream_ungetc(Stream*, char);

// Turn memory mode on
void stream_mark(Stream*);

// Recall all characters since memory mode was turned on
// The return value begins with the last character seen before stream_mark was called, and ends wit // the character before the last call to stream_getchar The next call to stream_getchar will repeat 
// the same character
char* stream_recall(Stream*);

// Free all stream resources, including the pointer itself
void stream_close(Stream*);
