#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"


void run_repl(ScamEnv*);
void run_debug_repl(ScamEnv*);


int main(int argc, char** argv) {
    ScamEnv* env = ScamEnv_builtins();
    char* cvalue = NULL;
    int load_flag = 0;
    int debug_flag = 0;
    int c;
    while ((c = getopt(argc, argv, "igc:")) != -1) {
        switch (c) {
            case 'i':
                load_flag = 1;
                break;
            case 'g':
                debug_flag = 1;
                break;
            case 'c':
                cvalue = optarg;
                ScamVal* v = eval_str(cvalue, env);
                ScamVal_println(v);
                return 0;
            case '?':
                return 1;
            default:
                break;
        }
    }
    /* Evaluate files. */
    for (int i = optind; i < argc; i++) {
        ScamVal* v = eval_file(argv[i], env);
        if (v->type == SCAM_ERR) {
            ScamVal_println(v);
        }
        gc_unset_root(v);
    }
    if (load_flag || debug_flag || argc == 1) {
        if (debug_flag) {
            run_debug_repl(env);
        } else {
            run_repl(env);
        }
    }
    gc_unset_root((ScamVal*)env);
    gc_close();
    return 0;
}

bool non_empty(const char* string) {
    for (; *string != '\0'; string++) {
        if (!isspace(*string)) {
            return true;
        }
    }
    return false;
}

void run_repl(ScamEnv* env) {
    while (1) {
        char* input = readline(">>> ");
        if (non_empty(input)) {
            add_history(input);
            if (strcmp(input, "quit") == 0) {
                free(input);
                break;
            }
            ScamVal* v = eval_str(input, env);
            ScamVal_println(v);
            gc_unset_root(v);
        }
        free(input);
    }
}

void parse_repl(char*);
void eval_repl(char*, ScamEnv*);

void print_generic_help(void);

enum { REPL_EVAL, REPL_PARSE };
void run_debug_repl(ScamEnv* env) {
    int mode = REPL_EVAL;
    print_generic_help();
    for (;;) {
        switch (mode) {
            case REPL_PARSE: printf("parse"); break;
            case REPL_EVAL: printf("eval"); break;
            default: printf("unknown mode"); break;
        }
        char* input = readline(">>> ");
        if (non_empty(input)) {
            add_history(input);
            if (strcmp(input, "quit") == 0) {
                free(input);
                break;
            } else if (strcmp(input, "!eval") == 0) {
                mode = REPL_EVAL;
            } else if (strcmp(input, "!parse") == 0) {
                mode = REPL_PARSE;
            } else {
                if (mode == REPL_PARSE) {
                    parse_repl(input);
                } else if (mode == REPL_EVAL) {
                    eval_repl(input, env);
                }
            }
        }
        free(input);
    }
}

void parse_repl(char* command) {
    if (strstr(command, "open") == command && strlen(command) >= 6) {
        ScamSeq* ast = parse_file(command + 5);
        ScamVal_print_ast((ScamVal*)ast, 0);
        gc_unset_root((ScamVal*)ast);
    } else if (strcmp(command, "help") == 0) {
        print_generic_help();
        puts("Parser commands:");
        puts("\topen <file path>: open a file for parsing");
        puts("\nAny other input is parsed and printed");
    } else {
        ScamSeq* ast = parse_str(command);
        ScamVal_print_ast((ScamVal*)ast, 0);
        gc_unset_root((ScamVal*)ast);
    }
}

void eval_repl(char* command, ScamEnv* env) {
    if (strcmp(command, "heap") == 0) {
        gc_smart_print();
    } else if (strcmp(command, "heapall") == 0) {
        gc_print();
    } else if (strcmp(command, "collect") == 0) {
        gc_collect();
    } else if (strcmp(command, "help") == 0) {
        print_generic_help();
        puts("Evaluator commands:");
        puts("\theap: print some objects in the heap (the interesting ones)");
        puts("\theapall: print all objects in the heap");
        puts("\tcollect: invoke the garbage collector");
        puts("\nAny other input is evaluated normally and printed");
    } else {
        ScamVal* v = eval_str(command, env);
        ScamVal_println(v);
        gc_unset_root(v);
    }
}

void print_generic_help(void) {
    puts("Universal commands:");
    puts("\thelp: print a help message");
    puts("\t!parse: switch to parse mode");
    puts("\t!eval: switch to evaluate mode");
    puts("\tquit: exit the program");
}
