#pragma once

#define MY_ASSERT(cond) if (!cond) return 0;

void stream_tests();
void tokenize_tests();
void parse_tests();
