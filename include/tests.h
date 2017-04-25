#pragma once

#define TEST_ASSERT(cond) \
    if (!(cond)) { \
        fprintf(stderr, "failure at %s:%d\n", __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    }

void stream_tests(void);
void tokenize_tests(void);
void parse_tests(void);
