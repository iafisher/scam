#pragma once
#include "scamval.h"

// Allocate a new scamval from the garbage collector's internal heap
scamval* gc_new_scamval();
// Create a copy of a scamval (this doesn't always result in an actual copy
// in memory, but if the proper API from scamval.h is used, then the user
// shouldn't know the difference)
scamval* gc_copy_scamval(scamval*);
// Relinquish control of a pointer (i.e., call this when you're done with a
// pointer, and don't try to use that pointer afterwards)
void gc_unset_root(scamval*);

// Invoke the garbage collector manually (generally unnecessary)
void gc_collect();
// Close the garbage collector and free all objects (obviously don't do this
// until the very end of the program, as all remaining refs become invalid)
void gc_close();

// Print the contents of the heap
void gc_print();
// Print the contents of the heap without builtin functions or names
void gc_smart_print();
