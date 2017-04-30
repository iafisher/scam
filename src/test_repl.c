#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "collector.h"
#include "eval.h"
#include "parse.h"

void parse_repl(char*);
void eval_repl(char*, scamval*);

void print_generic_help(void);

enum { REPL_EVAL, REPL_PARSE };
int main(int argc, char** argv) {
    // Run the REPL
    int mode = REPL_EVAL;
    char* buffer = NULL;
    size_t s_len = 0;
    scamval* env = scamdict_builtins();
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
    gc_close();
    return 0;
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
