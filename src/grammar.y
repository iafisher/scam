%output "grammar.c"
%defines "grammar.h"
%define api.pure full
%lex-param {void* scanner}
%parse-param {void* scanner} {scamval** out}

%code requires {
#include "scamval.h"
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif
}

%code {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scamval.h"
#include "flex.h"
int yyerror(yyscan_t scanner, scamval** out, const char* msg);

scamval* bison_parse_str(char*);
scamval* bison_parse_file(char*);
}

%union {
    long long ival;
    double fval;
    char* sval;
    scamval* nodeval;
}

%token DEFINE TRUE FALSE
%token <ival> INT
%token <fval> FLOAT
%token <sval> STRING SYMBOL
%type <nodeval> program block define_variable define_function expression expression_plus symbol_list symbol_plus statement_or_expression symbol value expression_star dictionary_item dictionary_list

%%
program:
    block { *out = $1; }
    ;
block:
    block statement_or_expression { $$ = $1; scamseq_append($$, $2); }
    | statement_or_expression { $$ = scamsexpr_from_vals(2, scamsym("begin"), $1); }
    ;
statement_or_expression:
    define_variable
    | define_function 
    | expression
    ;
define_variable:
    '(' DEFINE symbol expression ')' {
        $$ = scamsexpr_from_vals(3, scamsym("define"), $3, $4); 
    }
    ;
define_function:
    '(' DEFINE symbol_list block ')' {
        scamval* name = scamseq_pop($3, 0);
        scamval* lambda = scamsexpr_from_vals(3, scamsym("lambda"), $3, $4);
        $$ = scamsexpr_from_vals(3, scamsym("define"), name, lambda);
    }
    ;
symbol_list:
    '(' symbol_plus ')' { $$ = $2; }
    ;
symbol_plus:
    symbol_plus symbol { $$ = $1; scamseq_append($$, $2); }
    | symbol { $$ = scamsexpr_from_vals(1, $1); }
    ;
expression:
    value
    | symbol
    | '(' expression_plus ')' { $$ = $2; } 
    | '(' ')' { $$ = scamsexpr(); }
    ;
expression_star:
    expression_star expression { $$ = $1; scamseq_append($$, $2); }
    | { $$ = scamsexpr(); }
    ;
expression_plus:
    expression_star expression { $$ = $1; scamseq_append($$, $2); }
symbol:
    SYMBOL { $$ = scamsym_no_copy($1); }
    ;
value:
    INT { $$ = scamint($1); }
    | FLOAT { $$ = scamdec($1); }
    | STRING { $$ = scamstr_escapes($1); }
    | TRUE { $$ = scambool(1); }
    | FALSE { $$ = scambool(0); }
    | '[' expression_star ']' { $$ = $2; $$->type = SCAM_LIST; }
    | '{' dictionary_list '}' { $$ = $2; scamseq_prepend($$, scamsym("dict")); }
    ;
dictionary_list:
    dictionary_list dictionary_item { $$ = $1; scamseq_append($$, $2); }
    | { $$ = scamsexpr(); }
    ;
dictionary_item:
    expression ':' expression { 
        $$ = scamsexpr_from_vals(2, $1, $3); 
        $$->type = SCAM_LIST;
    }
    ;
%%

int yyerror(yyscan_t scanner, scamval** out, const char* s) {
    scamval* ret = scamerr(s);
    *out = ret;
    return 0;
}

// Note that this function will close the file when it's done
static scamval* parse_file_helper(FILE*);

scamval* parse_str(char* s) {
    return parse_file_helper(fmemopen(s, strlen(s), "r"));
}

scamval* parse_file(char* fp) {
    return parse_file_helper(fopen(fp, "r"));
}

static scamval* parse_file_helper(FILE* fsock) {
    if (fsock) {
        scamval* ret = NULL;
        yyscan_t myscanner;
        yylex_init(&myscanner);
        yyset_in(fsock, myscanner);
        yyparse(myscanner, &ret);
        yylex_destroy(myscanner);
        fclose(fsock);
        return ret ? ret : scamerr("failed to generate parse tree");
    } else {
        return scamerr("unable to open file");
    }
}
