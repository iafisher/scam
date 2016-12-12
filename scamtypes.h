#pragma once
#include "scamval.h"

// Return the names of types as strings
const char* scamtype_name(int type);
const char* scamtype_debug_name(int type);

// Check if the scamvalue belongs to the given type
int scamval_typecheck(scamval*, int type);

// Return the narrowest type applicable to both types
int narrowest_type(int, int);

// Return the narrowest type applicable to all elements of the sequence
int scamseq_narrowest_type(scamval*);

// Construct a type error message
scamval* scamerr_type(const char* name, size_t pos, int got, int expected);
