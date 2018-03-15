#include <stdio.h>
#include "collector.h"
#include "scamval.h"


typedef enum { CODE_ADD } bytecode_t;


typedef struct {
    bytecode_t inst;
    ScamVal* arg1, *arg2;
} codeobj;


char* codeobj_to_str(const codeobj*);

codeobj* ScamVal_compile(const ScamVal*);


int main(int argc, char* argv[]) {
    puts(codeobj_to_str(ScamVal_compile(NULL)));
    return 0;
}


static const char* bytecode_to_str(bytecode_t inst) {
    switch (inst) {
        case CODE_ADD:
            return "CODE_ADD";
        default:
            return "unknown bytecode instruction";
    }
}


char* codeobj_to_str(const codeobj* code) {
    char* ret;
    size_t n;
    FILE* stream = open_memstream(&ret, &n);
    fputs(bytecode_to_str(code->inst), stream);
    if (code->arg1)
        ScamVal_write(code->arg1, stream);
    if (code->arg2)
        ScamVal_write(code->arg2, stream);
    fclose(stream);
    return ret;
}


codeobj* ScamVal_compile(const ScamVal* v) {
    codeobj* ret = gc_malloc(sizeof *ret);
    ret->inst = CODE_ADD;
    ret->arg1 = ret->arg2 = NULL;
    return ret;
}
