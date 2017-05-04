#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"
#include "scamval.h"


/* Assert that the condition is true, printing the error message if it is not */
#define ASSERT(cond, ps, msg) \
    if (!(cond)) { \
        printf("Error at line %d in %s: %s\n", ps.line_no, ps.fpath, msg); \
    }

/* Assert that the answer string is equal to the given string, printing an error message if it is
 * not
 */
#define ASSERT_EQ(answer_str, this_str, ps) \
    if (strcmp(this_str, answer_str) != 0) { \
        printf("Failed example at line %d in %s:\n", ps.line_no, ps.fpath); \
        printf("  %s\n", ps.query + 3); \
        printf("Expected:\n"); \
        printf("  %s\n", answer_str); \
        printf("Got:\n"); \
        printf("  %s\n", this_str); \
    }

typedef struct {
    int line_no;
    FILE* fsock;
    char* fpath;
    char* query;
    char* answer;
    size_t query_n, answer_n; // lengths of lines, used for getline
} program_state_t;

void ps_init(program_state_t*, const char* file_path);
void ps_free(program_state_t*);
// Read the next pair of lines from the file, returning 0 if EOF was reached and 1 otherwise
int ps_next(program_state_t*);

int is_good_line(const char*);
int is_query(const char*);
ssize_t get_good_line(char**, size_t*, FILE*, int* line_no);

int main(int argc, char* argv[]) {
    if (argc == 2) {
        program_state_t ps;
        ps_init(&ps, argv[1]);
        ASSERT(ps.fsock != NULL, ps, "unable to open file")
        scamval* env = scamdict_builtins();
        while (ps_next(&ps)) {
            ASSERT(is_query(ps.query), ps, "expected \">>> ...\"")
            scamval* query_value = eval_str(ps.query + 3, env);
            char* correct_str = scamval_to_repr(query_value);
            char* this_str = is_query(ps.answer) ? "" : ps.answer;
            if (strcmp(ps.answer, "ERROR") == 0) {
                ASSERT(query_value->type == SCAM_ERR, ps, "expected error")
            } else {
                ASSERT_EQ(correct_str, this_str, ps);
            }
            free(correct_str);
        }
        ps_free(&ps);
        return 0;
    } else {
        return 2;
    }
}

void ps_init(program_state_t* ps, const char* file_path) {
    if (!ps) return;
    ps->line_no = 0;
    ps->fsock = fopen(file_path, "r");
    ps->fpath = strdup(file_path);
    ps->query = NULL;
    ps->answer = NULL;
    ps->query_n = 0;
    ps->answer_n = 0;
}

void ps_free(program_state_t* ps) {
    if (ps->fsock) fclose(ps->fsock);
    if (ps->fpath) free(ps->fpath);
    if (ps->query) free(ps->query);
    if (ps->answer) free(ps->answer);
    gc_close();
}

int ps_next(program_state_t* ps) {
    if (is_query(ps->answer)) {
        free(ps->query);
        ps->query = strdup(ps->answer);
        ps->query_n = strlen(ps->answer) + 1;
        ssize_t result = get_good_line(&ps->answer, &ps->answer_n, ps->fsock, &ps->line_no);
        return (result == -1) ? 0 : 1;
    }  else {
        ssize_t result = get_good_line(&ps->query, &ps->query_n, ps->fsock, &ps->line_no);
        if (result == -1) return 0;
        result = get_good_line(&ps->answer, &ps->answer_n, ps->fsock, &ps->line_no);
        return (result == -1) ? 0 : 1;
    }
}

int is_good_line(const char* line) {
    if (!line) return 0;
    for (; *line != '\0'; line++) {
        if (*line == ';') return 0;
        if (!isspace(*line)) return 1;
    }
    return 0;
}

int is_query(const char* line) {
    return line != NULL && strlen(line) > 3 && (strncmp(line, ">>>", 3) == 0);
}

ssize_t get_good_line(char** lineptr, size_t* n, FILE* stream, int* line_no) {
    ssize_t result = -1;
    do {
        result = getline(lineptr, n, stream);
        if (result == -1) return result;
        // remove the terminating newline
        (*lineptr)[result - 1] = '\0';
        if (line_no) (*line_no)++;
    } while (!is_good_line(*lineptr));
    return result;
}
