#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "collector.h"
#include "eval.h"
#include "parse.h"

void run_repl(scamval*);
void run_debug_repl(scamval*);

int main(int argc, char** argv) {
    scamval* env = scamdict_builtins();
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
    if (load_flag || debug_flag || argc == 1) {
        if (debug_flag) {
            run_debug_repl(env);
        } else {
            run_repl(env);
        }
    }
    gc_unset_root(env);
    gc_close();
    return 0;
}

void run_repl(scamval* env) {
    while (1) {
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
}

void parse_repl(char*);
void eval_repl(char*, scamval*);

void print_generic_help(void);

enum { REPL_EVAL, REPL_PARSE };
void run_debug_repl(scamval* env) {
    // Run the REPL
    int mode = REPL_EVAL;
    char* buffer = NULL;
    size_t s_len = 0;
    print_generic_help();
    for (;;) {
        switch (mode) {
            case REPL_PARSE: printf("parse"); break;
            case REPL_EVAL: printf("eval"); break;
            default: printf("unknown mode"); break;
        }
        printf(">>> ");
        int end = getline(&buffer, &s_len, stdin);
        // remove trailing newline
        if (end > 0) {
            buffer[--end] = '\0';
        }
        if (strcmp(buffer, "quit") == 0) {
            break;
        } else if (strcmp(buffer, "!eval") == 0) {
            mode = REPL_EVAL;
            continue;
        } else if (strcmp(buffer, "!parse") == 0) {
            mode = REPL_PARSE;
            continue;
        }
        if (mode == REPL_PARSE) {
            parse_repl(buffer);
        } else if (mode == REPL_EVAL) {
            eval_repl(buffer, env);
        }
    }
    if (buffer) free(buffer);
}

void parse_repl(char* command) {
    if (strstr(command, "open") == command && strlen(command) >= 6) {
        scamval* ast = parse_file(command + 5);
        scamval_print_ast(ast, 0);
        gc_unset_root(ast);
    } else if (strcmp(command, "help") == 0) {
        print_generic_help();
        puts("Parser commands:");
        puts("\topen <file path>: open a file for parsing");
        puts("\nAny other input is parsed and printed");
    } else {
        //scamval* ast = parse_str(command);
        scamval* ast = parse_str(command);
        scamval_print_ast(ast, 0);
        gc_unset_root(ast);
    }
}

void eval_repl(char* command, scamval* env) {
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
        scamval* v = eval_str(command, env);
        scamval_println(v);
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
