#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../collector.h"
#include "../eval.h"
#include "../scamval.h"

// values for the 'expect' variable in the while loop
enum { QUERY, ANSWER };

#define ASSERT(cond, msg, line) \
    if (!(cond)) { \
        printf("Error at line %d: %s\n", line, msg); \
        gc_close(); \
        return 1; \
    }

#define ASSERT_EQ(this_val, last_val, line) \
    if (!scamval_eq(this_val, last_val)) { \
        printf("Error at line %d: got ", line); \
        scamval_print_debug(last_val); \
        printf(", expected "); \
        scamval_print_debug(this_val); \
        printf("\n"); \
        gc_close(); \
        return 2; \
    }

int main(int argc, char* argv[]) {
    if (argc == 2) {
        int line = 0;
        FILE* fp = fopen(argv[1], "r");
        ASSERT(fp != NULL, "unable to open file", line);
        // inputs to getline
        char* buffer = NULL; size_t n = 0;
        scamval* last_val = scamnull();
        scamval* env = scamdict_builtins();
        int expect = QUERY;
        while (getline(&buffer, &n, fp) != -1) {
            if (strncmp(buffer, ";", 1) == 0) {
                // skip comments
                ;
            } else if (strncmp(buffer, ">>>", 3) == 0) {
                ASSERT(expect == QUERY, "expected query, got answer", line);
                gc_unset_root(last_val);
                last_val = eval_str(buffer + 3, env);
                if (last_val->type != SCAM_NULL)
                    expect = ANSWER;
            } else {
                ASSERT(expect == ANSWER, "expected answer, got query", line);
                if (strcmp(buffer, "ERROR\n") == 0) {
                    ASSERT(last_val->type == SCAM_ERR, "expected error", 
                           line);
                } else {
                    scamval* this_val = eval_str(buffer, env);
                    ASSERT_EQ(this_val, last_val, line);
                    gc_unset_root(this_val);
                }
                expect = QUERY;
            }
            line++;
        }
        ASSERT(expect == QUERY, "expected final answer", line);
        free(buffer);
        gc_close();
        return 0;
    } else {
        return 2;
    }
}
