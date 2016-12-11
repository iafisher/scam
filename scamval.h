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

typedef struct {
    size_t count;
    size_t mem_size;
    scamval** root;
} array;

// Other convenient typedefs
typedef scamval* (scambuiltin_t)(scamval*);
typedef FILE scamport_t;

// Basic array functions
array* array_init();
array* array_copy(const array*);
void array_free(array*);
size_t array_len(const array*);

// Return a copy of the given array element (do not free this value!)
scamval* array_get(const array*, size_t);

// Return the given array element (the caller is responsible for freeing it)
scamval* array_pop(array*, size_t);

// Set the given array element WITHOUT freeing what was previously there
void array_set(array*, size_t, scamval*);

// The caller of these functions surrenders control of the given value to the
// array, so don't free a value after appending or prepending it!
void array_append(array*, scamval*);
void array_prepend(array*, scamval*);

// Useful wrapper functions around array methods
scamval* scamval_get(const scamval*, size_t);
scamval* scamval_pop(scamval*, size_t);
void scamval_set(scamval*, size_t, scamval*);
// same as scamval_set, except the previous element is free'd
void scamval_replace(scamval*, size_t, scamval*);
size_t scamval_len(const scamval*);
void scamval_append(scamval* seq, scamval*);
void scamval_prepend(scamval* seq, scamval*);

size_t scamval_strlen(const scamval*);

typedef struct {
    scamenv* env;
    scamval* parameters;
    scamval* body;
} scamfun_t;

struct scamval {
    int type;
    union {
        long long n; // used by SCAM_INT and SCAM_BOOL
        double d; // SCAM_DEC
        char* s; // SCAM_STR, SCAM_SYM and SCAM_ERR
        array* arr; // SCAM_LIST and SCAM_SEXPR
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

enum {SCAMENV_NORMAL, SCAMENV_TMP, SCAMENV_CLOSURE};
struct scamenv {
    // type determines how the environment is freed 
    int type;
    scamenv* enclosing;
    array* syms;
    array* vals;
};

scamenv* scamenv_init(scamenv* enclosing);
void scamenv_bind(scamenv*, scamval*, scamval*);
scamval* scamenv_lookup(scamenv*, scamval*);
void scamenv_free(scamenv*);
