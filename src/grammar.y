%output "grammar.c"
%defines "grammar.h"
%define api.pure full
%lex-param {void* scanner}
%parse-param {void* scanner} {ScamVal** out}

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
int yyerror(yyscan_t scanner, ScamVal** out, const char* msg);

ScamVal* bison_parse_str(char*);
ScamVal* bison_parse_file(char*);
}

%union {
    long long ival;
    double fval;
    char* sval;
    ScamVal* nodeval;
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
    block statement_or_expression { $$ = $1; ScamSeq_append((ScamSeq*)$$, $2); }
    | statement_or_expression { $$ = (ScamVal*)ScamExpr_from(2, ScamSym_new("begin"), $1); }
    ;
statement_or_expression:
    define_variable
    | define_function
    | expression
    ;
define_variable:
    '(' DEFINE symbol expression ')' {
        $$ = (ScamVal*)ScamExpr_from(3, ScamSym_new("define"), $3, $4);
    }
    ;
define_function:
    '(' DEFINE symbol_list block ')' {
        ScamVal* name = ScamSeq_pop((ScamSeq*)$3, 0);
        ScamVal* lambda = (ScamVal*)ScamExpr_from(3, ScamSym_new("lambda"), $3, $4);
        $$ = (ScamVal*)ScamExpr_from(3, ScamSym_new("define"), name, lambda);
    }
    ;
symbol_list:
    '(' symbol_plus ')' { $$ = $2; }
    ;
symbol_plus:
    symbol_plus symbol { $$ = $1; ScamSeq_append((ScamSeq*)$$, $2); }
    | symbol { $$ = (ScamVal*)ScamExpr_from(1, $1); }
    ;
expression:
    value
    | symbol
    | '(' expression_plus ')' { $$ = $2; }
    | '(' ')' { $$ = (ScamVal*)ScamExpr_new(); }
    ;
expression_star:
    expression_star expression { $$ = $1; ScamSeq_append((ScamSeq*)$$, $2); }
    | { $$ = (ScamVal*)ScamExpr_new(); }
    ;
expression_plus:
    expression_star expression { $$ = $1; ScamSeq_append((ScamSeq*)$$, $2); }
symbol:
    SYMBOL { $$ = (ScamVal*)ScamSym_no_copy($1); }
    ;
value:
    INT { $$ = (ScamVal*)ScamInt_new($1); }
    | FLOAT { $$ = (ScamVal*)ScamDec_new($1); }
    | STRING { $$ = (ScamVal*)ScamStr_from_literal($1); }
    | TRUE { $$ = (ScamVal*)ScamBool_new(1); }
    | FALSE { $$ = (ScamVal*)ScamBool_new(0); }
    | '[' expression_star ']' { $$ = $2; ScamSeq_prepend((ScamSeq*)$$, (ScamVal*)ScamSym_new("list")); }
    | '{' dictionary_list '}' { $$ = $2; ScamSeq_prepend((ScamSeq*)$$, (ScamVal*)ScamSym_new("dict")); }
    ;
dictionary_list:
    dictionary_list dictionary_item { $$ = $1; ScamSeq_append((ScamSeq*)$$, $2); }
    | { $$ = (ScamVal*)ScamExpr_new(); }
    ;
dictionary_item:
    expression ':' expression {
        $$ = (ScamVal*)ScamExpr_from(3, ScamSym_new("list"), $1, $3);
    }
    ;
%%

int yyerror(yyscan_t scanner, ScamVal** out, const char* s) {
    ScamVal* ret = (ScamVal*)ScamErr_new(s);
    *out = ret;
    return 0;
}

// Note that this function will close the file when it's done
static ScamVal* parse_file_helper(FILE*);

ScamVal* parse_str(char* s) {
    return parse_file_helper(fmemopen(s, strlen(s), "r"));
}

ScamVal* parse_file(char* fp) {
    return parse_file_helper(fopen(fp, "r"));
}

static ScamVal* parse_file_helper(FILE* fsock) {
    if (fsock) {
        ScamVal* ret = NULL;
        yyscan_t myscanner;
        yylex_init(&myscanner);
        yyset_in(fsock, myscanner);
        yyparse(myscanner, &ret);
        yylex_destroy(myscanner);
        fclose(fsock);
        return ret ? ret : (ScamVal*)ScamErr_new("failed to generate parse tree");
    } else {
        return (ScamVal*)ScamErr_new("unable to open file");
    }
}
