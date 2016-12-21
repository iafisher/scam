#include <stdio.h>
#include <stdlib.h>

void* my_malloc(size_t size) {
    void* ret = malloc(size);
    if (ret == NULL) {
        fputs("out of memory... exiting program\n", stderr);
        exit(EXIT_FAILURE);
    }
    return ret;
}

void* my_realloc(void* ptr, size_t size) {
    void* ret = realloc(ptr, size);
    if (ret == NULL) {
        fputs("out of memory... exiting program\n", stderr);
        exit(EXIT_FAILURE);
    }
    return ret;
}

void* my_calloc(size_t num, size_t size) {
    void* ret = calloc(num, size);
    if (ret == NULL) {
        fputs("out of memory... exiting program\n", stderr);
        exit(EXIT_FAILURE);
    }
    return ret;
}
