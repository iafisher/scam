#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "collector.h"
#include "eval.h"

void run_repl(scamval*);

int main(int argc, char** argv) {
    scamval* env = scamdict_builtins();
    char* cvalue = NULL;
    int load_flag = 0;
    int c;
    while ((c = getopt(argc, argv, "ic:")) != -1) {
        switch (c) {
            case 'i':
                load_flag = 1;
                break;
            case 'c':
                cvalue = optarg;
                scamval* v = eval_str(cvalue, env);
                scamval_println(v);
                return 0;
            case '?':
                return 1;
            default:
                break;
        }
    }
    // evaluate files
    for (int i = optind; i < argc; i++) {
        scamval* v = eval_file(argv[i], env);
        if (v->type == SCAM_ERR) {
            scamval_println(v);
        }
        gc_unset_root(v);
    }
    if (load_flag || argc == 1) {
        run_repl(env);
    }
    gc_unset_root(env);
    gc_close();
    return 0;
}

void run_repl(scamval* env) {
    /*
    char* buffer = NULL;
    size_t len = 0;
    */
    while (1) {
        /*
        printf(">>> ");
        getline(&buffer, &len, stdin);
        */
        char* input = readline(">>> ");
        add_history(input);
        if (strcmp(input, "quit") == 0) {
            free(input);
            break;
        }
        scamval* v = eval_str(input, env);
        scamval_println(v);
        gc_unset_root(v);
        free(input);
    }
    //free(buffer);
}
