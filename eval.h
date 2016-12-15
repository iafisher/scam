#pragma once
#include <stdio.h>
#include "scamval.h"

// Evaluate a Scam AST or value
scamval* eval(scamval*, scamenv*);

// Parse a string into a Scam AST and evaluate it
scamval* eval_str(char* s, scamenv*);

// Parse a file into a Scam AST and evaluate it
scamval* eval_file(char* fp, scamenv*);

// Evaluate a function application
scamval* eval_apply(scamval* fun, scamval* arglist);
