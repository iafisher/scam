#pragma once
#include <stdio.h>

#define SCAM_CLOSED_FILE -1

enum {SCAM_INT, SCAM_DEC, SCAM_BOOL, SCAM_LIST, SCAM_STR, SCAM_QUOTE,
      SCAM_FUNCTION, SCAM_PORT, SCAM_BUILTIN, SCAM_CODE, SCAM_SYM, SCAM_ERR,
      SCAM_NULL };

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
typedef scamval* (scambuiltin)(scamval*);
typedef FILE scamport;

array* array_init();
array* array_copy(array*);
void array_free(array*);

scamval* array_get(array*, size_t);
scamval* array_pop(array*, size_t);
void array_set(array*, size_t, scamval*);
size_t array_len(array*);
void array_append(array*, scamval*);
void array_prepend(array*, scamval*);

// Useful wrapper functions around array methods
scamval* scamval_get(scamval*, size_t);
scamval* scamval_pop(scamval*, size_t);
void scamval_set(scamval*, size_t, scamval*);
size_t scamval_len(scamval*);
void scamval_append(scamval*, scamval*);
void scamval_prepend(scamval*, scamval*);

typedef struct {
    scamenv* env;
    scamval* parameters;
    scamval* body;
} scamfun;

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
scamval* scamval_err(char*, ...);
scamval* scamval_function(scamenv*, scamval*, scamval*);
scamval* scamval_builtin(scambuiltin*);
scamval* scamval_null();

scamval* scamval_copy(scamval*);

const char* scamval_type_name(int type);
void scamval_print(scamval*);
void scamval_println(scamval*);

int scamval_eq(scamval*, scamval*);

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
scamenv* scamenv_copy(scamenv*);
void scamenv_free(scamenv*);
