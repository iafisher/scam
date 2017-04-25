#pragma once
#include <stdio.h>
#include "scamval.h"

// Evaluate a Scam AST or value
scamval* eval(scamval*, scamval*);

// Parse a string into a Scam AST and evaluate it
scamval* eval_str(char* s, scamval*);

// Parse a file into a Scam AST and evaluate it
scamval* eval_file(char* fp, scamval*);

// Evaluate a function application
scamval* eval_apply(scamval* fun, scamval* arglist);
