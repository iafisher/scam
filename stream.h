#pragma once

enum { STREAM_STR, STREAM_FILE };
typedef struct {
    int type;
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
Stream* stream_from_str(const char*);
Stream* stream_from_file(const char*);

// Return 1 if the stream can still be read from
int stream_good(Stream*);

// Get the current character from the stream and advance to the next one
char stream_getchar(Stream*);

// Put the last character back on the stream, effectively negating a previous
// call to getchar
// Putting characters is disabled in memory mode
void stream_putchar(Stream*);

// Turn memory mode on
void stream_mark(Stream*);

// Recall all characters since memory mode was turned on
char* stream_recall(Stream*);

// Free all stream resources, including the pointer itself
void stream_close(Stream*);
