#pragma once
#define SCAM_CLOSED_FILE -1

enum {SCAM_INT, SCAM_DEC, SCAM_BOOL, SCAM_LIST, SCAM_STR, SCAM_QUOTE,
      SCAM_FUNCTION, SCAM_PORT, SCAM_BUILTIN, SCAM_CODE, SCAM_SYM, SCAM_ERR };

// Forward declaration of scamval and scamenv
struct scamval;
typedef struct scamval scamval;
struct scamenv;
typedef struct scamenv scamenv;
// Other convenient typedefs
typedef scamval* (*scambuiltin)(scamval*);
typedef FILE scamport;

typedef struct {
    size_t count;
    scamval** root;
} array;

array* array_init();
void array_append(array*, scamval*);
array* array_copy(array*);
void array_free(array*);

typedef struct {
    scamenv* env;
    array* parameters;
    scamval* body;
} scamfun;

// Append an element (without copying) to a sequence
void scamval_append(scamval*, scamval*);

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
void scamenv_bind(scamenv*, char*, scamval*);
scamval* scamenv_lookup(scamenv*, char*);
void scamenv_free(scamenv*);
