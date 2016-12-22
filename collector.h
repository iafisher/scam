#pragma once
#include "scamval.h"

scamval* gc_new_scamval();
scamval* gc_copy_scamval(scamval*);
void gc_unset_root(scamval*);

void gc_collect();
void gc_close();

void gc_print();
