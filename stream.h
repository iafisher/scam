#pragma once

enum { STREAM_STR, STREAM_FILE };
typedef struct {
    int type;
    int line, col;
    int memory_flag; // -1 if not remembering, > -1 otherwise
    int memory_len;
    // used by string streams
    int s_len;
    char* s;
    // used by file streams
    int last_pos;
    FILE* fp;
} Stream;

// Initialize streams from various sources
Stream* stream_from_str(char*);
Stream* stream_from_file(char*);

// Return 1 if the stream can still be read from
int stream_good(Stream*);
// Get the current character from the stream and advance to the next one
char stream_getchar(Stream*);
// Begin remembering the characters
void stream_mark(Stream*);
// Return all remembered characters and clear the memory
char* stream_recall(Stream*);
// Free all stream resources, including the pointer itself
void stream_close(Stream*);
