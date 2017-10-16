#pragma once
#include <stdio.h>
#include "scamval.h"


/* Evaluate a Scam AST or value. */
ScamVal* eval(ScamVal*, ScamEnv*);


/* Parse a string into a Scam AST and evaluate it. */
ScamVal* eval_str(char* s, ScamEnv*);


/* Parse a file into a Scam AST and evaluate it. */
ScamVal* eval_file(char* fp, ScamEnv*);


/* Evaluate a function application. */
ScamVal* eval_apply(ScamVal* fun, ScamSeq* arglist);
