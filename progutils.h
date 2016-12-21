#pragma once
// Wrappers that exit the program if allocation fails
void* my_malloc(size_t);
void* my_realloc(void*, size_t);
void* my_calloc(size_t, size_t);
