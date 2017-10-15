#pragma once
#include <stdio.h>
#include "scamval.h"


/* Evaluate a Scam AST or value. */
ScamVal* eval(ScamVal*, ScamDict* env);


/* Parse a string into a Scam AST and evaluate it. */
ScamVal* eval_str(char* s, ScamDict* env);


/* Parse a file into a Scam AST and evaluate it. */
ScamVal* eval_file(char* fp, ScamDict* env);


/* Evaluate a function application. */
ScamVal* eval_apply(ScamVal* fun, ScamSeq* arglist);
