#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "eval.h"

void run_repl(scamenv*);

int main(int argc, char** argv) {
    scamenv* env = scamenv_init(NULL);
    register_builtins(env);
    int load_flag = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            load_flag = 1;
        } else {
            scamval* v = eval_file(argv[i], env);
            if (v->type == SCAM_ERR) {
                scamval_println(v);
            }
            scamval_free(v);
        }
    }
    if (load_flag || argc == 1) {
        run_repl(env);
    }
    scamenv_free(env);
    return 0;
}

void run_repl(scamenv* env) {
    char* buffer = NULL;
    size_t len = 0;
    while (1) {
        printf(">>> ");
        getline(&buffer, &len, stdin);
        if (strcmp(buffer, "quit\n") == 0) break;
        scamval* v = eval_str(buffer, env);
        scamval_println(v);
        scamval_free(v);
    }
    if (buffer) {
        free(buffer);
    }
}
