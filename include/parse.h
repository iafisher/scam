#pragma once
#include <stdio.h>
#include "scamval.h"


/* Parse a string and return either an AST or an error. */
scamval* old_parse_str(char* s);
scamval* parse_str(char* s);


/* Parse a file given its path and return either an AST or an error. */
scamval* old_parse_file(char* fp);
scamval* parse_file(char* fp);
