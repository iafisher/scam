#pragma once
#include <stdio.h>

// Possible values for the type field of the scamval struct
// Note that some of these types are never exposed to the user
enum {SCAM_INT, SCAM_DEC, SCAM_BOOL, SCAM_LIST, SCAM_STR, SCAM_FUNCTION, 
      SCAM_PORT, SCAM_BUILTIN, SCAM_SEXPR, SCAM_SYM, SCAM_ERR, SCAM_NULL };

const char* scamtype_name(int type);
const char* scamtype_debug_name(int type);

// Forward declaration of scamval and scamenv
struct scamval;
typedef struct scamval scamval;
struct scamenv;
typedef struct scamenv scamenv;

// Other convenient typedefs
typedef scamval* (scambuiltin_t)(scamval*);
typedef FILE scamport_t;

// Useful functions for lists and S-expressions
scamval* scamseq_get(const scamval*, size_t);
scamval* scamseq_pop(scamval*, size_t);
void scamseq_set(scamval*, size_t, scamval*);
// same as scamval_set, except the previous element is free'd
void scamseq_replace(scamval*, size_t, scamval*);
size_t scamseq_len(const scamval*);
void scamseq_append(scamval* seq, scamval*);
void scamseq_prepend(scamval* seq, scamval*);

size_t scamstr_len(const scamval*);

typedef struct {
    scamenv* env;
    scamval* parameters;
    scamval* body;
} scamfun_t;

struct scamval {
    int type;
    size_t count, mem_size; // used by SCAM_LIST, SCAM_SEXPR and SCAM_STR
    union {
        long long n; // used by SCAM_INT and SCAM_BOOL
        double d; // SCAM_DEC
        char* s; // SCAM_STR, SCAM_SYM and SCAM_ERR
        //array* arr; // SCAM_LIST and SCAM_SEXPR
        scamval** arr;
        scamfun_t* fun; // SCAM_FUNCTION
        scamport_t* port; // SCAM_PORT
        scambuiltin_t* bltin; // SCAM_BUILTIN
    } vals;
};

// Make scamvals out of various C types
scamval* scamint(long long);
scamval* scamdec(double);
scamval* scambool(int);
scamval* scamlist();
scamval* scamcode();
scamval* scamport(FILE*);
scamval* scamstr(const char*);
scamval* scamstr_n(const char*, size_t n);
scamval* scamstr_from_char(char);
scamval* scamsym(const char*);
scamval* scamerr(const char*, ...);
scamval* scamfunction(scamenv*, scamval* parameters, scamval* body);
scamval* scambuiltin(scambuiltin_t*);
scamval* scamnull();

// Useful error message constructors
scamval* scamerr_arity(const char* name, size_t got, size_t expected);
scamval* scamerr_min_arity(const char* name, size_t got, size_t expected);
scamval* scamerr_type(const char* name, size_t pos, int given_type, int req_type);
scamval* scamerr_type2(const char* name, size_t pos, int given_type);

scamval* scamval_copy(const scamval*);

void scamval_print(const scamval*);
void scamval_println(const scamval*);
void scamval_print_debug(const scamval*);
void scamval_print_ast(const scamval*, int indent);

int scamval_eq(const scamval*, const scamval*);

// Free all resources used by a scamval, including the pointer itself
void scamval_free(scamval*);

struct scamenv {
    int references;
    scamenv* enclosing;
    // symbols and values are stored as scamval lists
    scamval* syms;
    scamval* vals;
};

scamenv* scamenv_init(scamenv* enclosing);
void scamenv_bind(scamenv*, scamval*, scamval*);
scamval* scamenv_lookup(scamenv*, scamval*);
void scamenv_free(scamenv*);
