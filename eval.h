#pragma once
#include <stdio.h>
#include "scamval.h"

// Evaluate a Scam abstract syntax tree or value
scamval* eval(scamval*, scamenv*);
scamval* eval_line(char*, scamenv*);
scamval* eval_file(char*, scamenv*);
