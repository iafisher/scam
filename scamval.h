#pragma once
#include <stdio.h>

// Possible values for the type field of the scamval struct
// Note that some of these types are never exposed to the user
enum {SCAM_INT, SCAM_DEC, SCAM_BOOL, SCAM_LIST, SCAM_STR, SCAM_LAMBDA, 
      SCAM_PORT, SCAM_BUILTIN, SCAM_SEXPR, SCAM_SYM, SCAM_ERR, SCAM_NULL};

// Type values that are only used for typechecking
enum {SCAM_SEQ=1000, SCAM_NUM, SCAM_CMP, SCAM_FUNCTION, SCAM_ANY};

// Forward declaration of scamval and scamenv
struct scamval;
typedef struct scamval scamval;
struct scamenv;
typedef struct scamenv scamenv;

// Another convenient typedef
typedef scamval* (scambuiltin_t)(scamval*);

// Return a reference to the i'th element of the sequence
// Make sure not to free this reference!
scamval* scamseq_get(const scamval*, size_t i);

// Remove and return the i'th element of the sequence
// The caller assumes responsibility for freeing the value
scamval* scamseq_pop(scamval*, size_t i);

// Set the i'th element of the sequence, obliterating the old element without
// freeing it (DO NOT USE unless you know the i'th element is already free)
void scamseq_set(scamval* seq, size_t i, scamval* v);

// Same as scamval_set, except the previous element is free'd before
void scamseq_replace(scamval*, size_t, scamval*);

// Return the actual number of elements in the sequence or string
size_t scamseq_len(const scamval*);
size_t scamstr_len(const scamval*);

// Append/prepend a value to a sequence
// The sequence takes responsibility for freeing the value, so it's best not
// to use a value once you've appended or prepended it somewhere
void scamseq_append(scamval* seq, scamval* v);
void scamseq_prepend(scamval* seq, scamval* v);

// Concatenate the second argument to the first, freeing the second arg
void scamseq_concat(scamval* seq1, scamval* seq2);
void scamstr_concat(scamval* s1, scamval* s2);

// Free the internal sequence of a scamval, without freeing the actual value
void scamseq_free(scamval*);

typedef struct {
    scamenv* env; // the environment the function was created in
    scamval* parameters;
    scamval* body;
} scamfun_t;

enum { SCAMPORT_OPEN, SCAMPORT_CLOSED };
typedef struct {
    int status;
    FILE* fp;
} scamport_t;

struct scamval {
    int type;
    int line, col;
    size_t count, mem_size; // used by SCAM_LIST, SCAM_SEXPR and SCAM_STR
    union {
        long long n; // used by SCAM_INT and SCAM_BOOL
        double d; // SCAM_DEC
        char* s; // SCAM_STR, SCAM_SYM and SCAM_ERR
        scamval** arr; // SCAM_LIST and SCAM_SEXPR
        scamfun_t* fun; // SCAM_LAMBDA
        scamport_t* port; // SCAM_PORT
        scambuiltin_t* bltin; // SCAM_BUILTIN
    } vals;
    // accounting info for the garbage collector
    int refs;
};

// Make scamvals out of various C types
scamval* scamval_new(int type);
scamval* scamint(long long);
scamval* scamdec(double);
scamval* scambool(int);
scamval* scamlist();
scamval* scamsexpr();
scamval* scamsexpr_from_vals(size_t, ...);
scamval* scamport(FILE*);
scamval* scamstr(const char*);
// Create a string from the first n characters of the given string
scamval* scamstr_n(const char*, size_t n);
scamval* scamstr_from_char(char);
scamval* scamsym(const char*);
// Create a formatted error message
scamval* scamerr(const char*, ...);
scamval* scamfunction(scamenv* env, scamval* parameters, scamval* body);
scamval* scambuiltin(scambuiltin_t*);
scamval* scamnull();

// Useful error message constructors
scamval* scamerr_arity(const char* name, size_t got, size_t expected);
scamval* scamerr_min_arity(const char* name, size_t got, size_t expected);

// Return a copy of the given value
scamval* scamval_copy(scamval*);
scamval* scamval_new_ref(scamval*);
// Free all resources used by a scamval, including the pointer itself
void scamval_free(scamval*);

void scamval_print(const scamval*);
void scamval_println(const scamval*);
void scamval_print_debug(const scamval*);
void scamval_print_ast(const scamval*, int indent);

// Comparisons between scamvals
int scamval_eq(const scamval*, const scamval*);
int scamval_gt(const scamval*, const scamval*);

struct scamenv {
    scamenv* enclosing;
    // symbols and values are stored as scamval lists
    scamval* syms;
    scamval* vals;
    // garbage collection accounting
    int is_tmp;
};

// Initialize and free environments
scamenv* scamenv_init(scamenv* enclosing);
scamenv* scamenv_init_tmp(scamenv* enclosing);
void scamenv_free(scamenv*);
void scamenv_free_tmp(scamenv*);

// Create a new binding in the environment, or update an existing one
// sym should be of type SCAM_STR
// Both sym and val are appropriated by the environment, so don't use them
// after calling this function
void scamenv_bind(scamenv*, scamval* sym, scamval* val);

// Lookup the symbol in the environment, returning a copy of the value if it
// exists and an error if it doesn't
scamval* scamenv_lookup(scamenv*, scamval* sym);

// Wrappers that exit the program if allocation fails
void* my_malloc(size_t);
void* my_realloc(void*, size_t);
