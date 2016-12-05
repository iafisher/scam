#pragma once
#include <stdio.h>

#define SCAM_CLOSED_FILE -1

enum {SCAM_INT, SCAM_DEC, SCAM_BOOL, SCAM_LIST, SCAM_STR, SCAM_QUOTE,
      SCAM_FUNCTION, SCAM_PORT, SCAM_BUILTIN, SCAM_CODE, SCAM_SYM, SCAM_ERR };

const char* scamval_type_name(int type);

// Forward declaration of scamval and scamenv
struct scamval;
typedef struct scamval scamval;
struct scamenv;
typedef struct scamenv scamenv;

typedef struct {
    size_t count;
    scamval** root;
} array;

// Other convenient typedefs
typedef scamval* (scambuiltin)(array*);
typedef FILE scamport;

array* array_init();
void array_append(array*, scamval*);
array* array_copy(array*);
void array_free(array*);

typedef struct {
    scamenv* env;
    array* parameters;
    scamval* body;
} scamfun;

// Array utilities
void scamval_append(scamval*, scamval*);
scamval* scamval_get(scamval*, size_t);
size_t scamval_len(scamval*);

struct scamval {
    int type;
    union {
        long long n; // used by SCAM_INT and SCAM_BOOL
        double d; // SCAM_DEC
        char* s; // SCAM_STR, SCAM_SYM and SCAM_ERR
        array* arr; // SCAM_LIST, SCAM_QUOTE and SCAM_CODE
        scamfun* fun; // SCAM_FUNCTION
        scamport* port; // SCAM_PORT
        scambuiltin* bltin; // SCAM_BUILTIN
    } vals;
};

// Make scamvals out of various C types
scamval* scamval_int(long long);
scamval* scamval_dec(double);
scamval* scamval_bool(int);
scamval* scamval_list();
scamval* scamval_quote();
scamval* scamval_code();
scamval* scamval_port(FILE*);
scamval* scamval_str(char*);
scamval* scamval_sym(char*);
scamval* scamval_err(char*);
scamval* scamval_function();
scamval* scamval_builtin(scambuiltin*);

scamval* scamval_copy(scamval*);

void scamval_print(scamval*);
void scamval_println(scamval*);

// Free all resources used by a scamval, including the pointer itself
void scamval_free(scamval*);

struct scamenv {
    scamenv* enclosing;
    array* syms;
    array* vals;
};

scamenv* scamenv_init(scamenv* enclosing);
void scamenv_bind(scamenv*, scamval*, scamval*);
scamval* scamenv_lookup(scamenv*, scamval*);
void scamenv_free(scamenv*);
