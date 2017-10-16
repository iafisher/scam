#pragma once
#include <stdio.h>


/* The possible values for the type field of the ScamVal struct, populated with an X-macro.
 * Note that some of these types are never exposed to the user.
 */
enum ScamType {
#define EXPAND_TYPE(type_val, type_name) \
    type_val,
#include "../src/type.def"
};


#define SCAMVAL_HEADER \
    enum ScamType type; \
    /* Bookkeeping for the garbage collector. */ \
    int seen; \
    int is_root;


/* Used by SCAM_NULL, inherited by everything else. */
typedef struct {
    SCAMVAL_HEADER;
} ScamVal;


/* Used by SCAM_INT. */
typedef struct {
    SCAMVAL_HEADER;
    long long n;
} ScamInt;


/* Used by SCAM_DEC. */
typedef struct {
    SCAMVAL_HEADER;
    double d;
} ScamDec;


/* Used by SCAM_SEXPR, SCAM_LIST and SCAM_DOT_SYM. */
typedef struct {
    SCAMVAL_HEADER;
    size_t count, mem_size;
    ScamVal** arr;
} ScamSeq;


/* Used by SCAM_STR, SCAM_SYM and SCAM_ERR. */
typedef struct {
    SCAMVAL_HEADER;
    size_t count, mem_size;
    char* s;
} ScamStr;


typedef struct ScamDict_list {
    struct ScamDict_list* next;
    ScamVal* key;
    ScamVal* val;
} ScamDict_list;


/* Used by SCAM_DICT. */
enum { SCAM_DICT_SIZE = 256 };
typedef struct ScamDict_rec {
    SCAMVAL_HEADER;
    /* A pointer to the enclosing dictionary (if the dictionary is an environment). */
    struct ScamDict_rec* enclosing;
    size_t len;
    ScamDict_list* data[SCAM_DICT_SIZE];
} ScamDict;


/* Used by SCAM_FUNCTION. */
typedef struct {
    SCAMVAL_HEADER;
    ScamDict* env; /* A pointer to the environment the function was created in, for closures. */
    ScamSeq* parameters;
    ScamSeq* body;
} ScamFunction;


/* Used by SCAM_PORT. */
enum { SCAMPORT_OPEN, SCAMPORT_CLOSED };
typedef struct {
    SCAMVAL_HEADER;
    int status;
    FILE* fp;
} ScamPort;


/* Used by SCAM_BUILTIN. */
typedef ScamVal* (*scambuiltin_fun)(ScamSeq*);
typedef struct {
    SCAMVAL_HEADER;
    scambuiltin_fun fun;
    int constant;
} ScamBuiltin;



/*** SCAMVAL CONSTRUCTORS ***/
ScamStr* ScamSym_new(const char*);
ScamStr* ScamSym_no_copy(char*);
ScamVal* ScamNull_new(void);


/*** NUMERIC API ***/
ScamInt* ScamInt_new(long long);
ScamDec* ScamDec_new(double);
ScamInt* ScamBool_new(int);
long long ScamInt_unbox(const ScamInt*);
long long ScamBool_unbox(const ScamInt*);
double ScamDec_unbox(const ScamDec*);


/*** SEQUENCE API ***/
ScamSeq* ScamList_new(void);
ScamSeq* ScamList_from(size_t, ...);
ScamSeq* ScamExpr_new(void);
ScamSeq* ScamExpr_from(size_t, ...);

/* Return a reference to the i'th element of the sequence. */
ScamVal* ScamSeq_get(const ScamSeq*, size_t i);

/* Remove the i'th element of the sequence. */
void ScamSeq_delete(ScamSeq*, size_t i);

/* Remove and return the i'th element of the sequence. */
ScamVal* ScamSeq_pop(ScamSeq*, size_t i);

/* Set the i'th element of the sequence. 
 *   - This obliterates the old element without freeing it. 
 *   - DO NOT USE unless you know the i'th element is already free.
 */
void ScamSeq_set(ScamSeq* seq, size_t i, ScamVal* v);

/* Return the actual number of elements in the sequence. */
size_t ScamSeq_len(const ScamSeq*);

/* Append/prepend/insert a value into a sequence.
 *   - The sequence takes responsibility for freeing the value, so it's best not to use a value once
 *     you've inserted it somewhere.
 */
void ScamSeq_insert(ScamSeq* seq, size_t i, ScamVal* v);
void ScamSeq_append(ScamSeq* seq, ScamVal* v);
void ScamSeq_prepend(ScamSeq* seq, ScamVal* v);

/* Concatenate the second argument to the first.
 *   - The second argument is free'd.
 */
void ScamSeq_concat(ScamSeq* seq1, ScamSeq* seq2);

/* Return a newly allocated subsequence. */
ScamVal* ScamSeq_subseq(const ScamSeq* seq, size_t start, size_t end);


/*** STRING API ***/
/* Initialize a string from a character array by copying it. */
ScamStr* ScamStr_new(const char*);
/* Initialize a string from a character array without making a copy. */
ScamStr* ScamStr_no_copy(char*);
ScamStr* ScamStr_from_literal(char*);
ScamStr* ScamStr_empty(void);

/* Read a line from a file and return it as a string. */
ScamStr* ScamStr_read(FILE*);

ScamStr* ScamStr_from_char(char);
const char* ScamStr_unbox(const ScamStr*);
void ScamStr_set(ScamStr*, size_t, char);
void ScamStr_map(ScamStr*, int map_f(int));

/* Return the i'th character without removing it. */
char ScamStr_get(const ScamStr*, size_t i);

/* Remove and return the i'th character. */
char ScamStr_pop(ScamStr*, size_t i);
void ScamStr_remove(ScamStr*, size_t beg, size_t end);
void ScamStr_truncate(ScamStr*, size_t);

/* Return a newly-allocated substring. */
ScamStr* ScamStr_substr(const ScamStr*, size_t, size_t);
void ScamStr_concat(ScamStr* s1, ScamStr* s2);
size_t ScamStr_len(const ScamStr*);


/*** FUNCTION API ***/
ScamFunction* ScamFunction_new(ScamDict* env, ScamSeq* parameters, ScamSeq* body);
ScamBuiltin* ScamBuiltin_new(scambuiltin_fun);

/* Construct a constant scambuiltin (one that doesn't change its arguments). */
ScamBuiltin* ScamBuiltin_new_const(scambuiltin_fun);
size_t ScamFunction_nparams(const ScamFunction*);
ScamStr* ScamFunction_param(const ScamFunction*, size_t);
ScamSeq* ScamFunction_body(const ScamFunction*);

/* Initialize an environment enclosed by the function's environment. */
ScamDict* ScamFunction_env(const ScamFunction*);

/* Return a reference to the function's environment itself. */
const ScamDict* ScamFunction_env_ref(const ScamFunction*);
scambuiltin_fun ScamBuiltin_function(const ScamBuiltin*);
int ScamBuiltin_is_const(const ScamBuiltin*);


/*** ERROR API ***/
ScamStr* ScamErr_new(const char*, ...);
ScamStr* ScamErr_arity(const char* name, size_t got, size_t expected);
ScamStr* ScamErr_min_arity(const char* name, size_t got, size_t expected);
ScamStr* ScamErr_type(const char* name, size_t pos, int got, int expected);
ScamStr* ScamErr_eof();


/*** PORT API ***/
ScamPort* ScamPort_new(FILE*);
FILE* ScamPort_unbox(ScamPort*);
int ScamPort_status(const ScamPort*);
void ScamPort_set_status(ScamPort*, int);


/*** DICTIONARY API ***/
ScamDict* ScamDict_new(ScamDict* enclosing);
ScamDict* ScamDict_from(size_t, ...);
ScamDict* ScamDict_builtins(void);

/* Create a new binding in the dictionary, or update an existing one. */
void ScamDict_bind(ScamDict* dct, ScamVal* sym, ScamVal* val);

/* Lookup the symbol in the dictionary and return a copy of the value if it exists and an error if 
 * it doesn't.
 */
ScamVal* ScamDict_lookup(const ScamDict* dct, const ScamVal* sym);
size_t ScamDict_len(const ScamDict* dct);
ScamDict* ScamDict_enclosing(const ScamDict*);

void ScamDict_list_free(ScamDict_list*);


/*** SCAMVAL PRINTING ***/
char* ScamVal_to_repr(const ScamVal*);
char* ScamVal_to_str(const ScamVal*);
void ScamVal_print(const ScamVal*);
void ScamVal_println(const ScamVal*);
void ScamVal_print_debug(const ScamVal*);
void ScamVal_print_ast(const ScamVal*, int indent);


/*** SCAMVAL COMPARISONS ***/
int ScamVal_eq(const ScamVal*, const ScamVal*);
int ScamVal_gt(const ScamVal*, const ScamVal*);


/*** TYPECHECKING ***/
/* Return the names of types as strings. */
const char* scamtype_name(int type);
const char* scamtype_debug_name(int type);

/* Check if the value belongs to the given type. */
int ScamVal_typecheck(const ScamVal*, int type);

/* Return the narrowest type applicable to both types. */
int narrowest_type(int, int);

/* Return the narrowest type applicable to all elements of the sequence. */
int ScamSeq_narrowest_type(ScamSeq*);
