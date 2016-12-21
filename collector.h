#pragma once
#include "scamval.h"

void gc_unset_root(scamval*);
scamval* gc_new_scamval();
scamval* gc_copy_scamval(scamval*);
void gc_del_scamval(scamval*);

void gc_collect();
void gc_close();

void gc_print();
