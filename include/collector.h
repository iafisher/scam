#pragma once
#include "scamval.h"


/* Allocate a new ScamVal from the garbage collector's internal heap. */
ScamVal* gc_new_ScamVal(int type, size_t sz);


#define SCAMVAL_NEW(name, typ, typ_as_int) \
    typ* name = (typ*)gc_new_ScamVal(typ_as_int, sizeof *name);


/* Create a copy of a ScamVal (this doesn't always result in an actual copy in memory, but if the 
 * proper API from ScamVal.h is used, then the user shouldn't know the difference).
 */
ScamVal* gc_copy_ScamVal(ScamVal*);

/* Relinquish control of a pointer (i.e., call this when you're done with a pointer, and don't try 
 * to use that pointer afterwards).
 */
void gc_unset_root(ScamVal*);

/* Opposite of gc_unset_root, used internally by some of the sequence APIs. */
void gc_set_root(ScamVal*);


/* Invoke the garbage collector manually (generally unnecessary). */
void gc_collect(void);

/* Close the garbage collector and free all objects (obviously don't do this until the very end of 
 * the program, as all remaining refs become invalid).
 */
void gc_close(void);


/* Print the contents of the heap. */
void gc_print(void);

/* Print the contents of the heap without builtin functions or names. */
void gc_smart_print(void);


// Allocate and reallocate from the actual program heap, handling out of memory errors gracefully
void* gc_malloc(size_t);
void* gc_realloc(void*, size_t);
void* gc_calloc(size_t, size_t);
