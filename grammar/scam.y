%{
#include <stdlib.h>
#include "scamval.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern int line_num;
extern int col_num;

void yyerror(const char* s);

scamval* root;
%}

%code requires {
#include "scamval.h"
}

%define parse.lac full
%define parse.error verbose

%union {
    long long ival;
    double fval;
    char* sval;
    scamval* nodeval;
}

%token DEFINE
%token <ival> INT
%token <fval> FLOAT
%token <sval> STRING SYMBOL
%type <nodeval> program block define_variable define_function expression expression_plus symbol_list symbol_plus statement_or_expression symbol value expression_star dictionary_item dictionary_list

%%
program:
    block { root = $1; }
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
        scamval* lambda = scamsexpr_from_vals(2, scamsym("lambda"), $4);
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
    SYMBOL { $$ = scamsym($1); }
    ;
value:
    INT { $$ = scamint($1); }
    | FLOAT { $$ = scamdec($1); }
    | STRING { $$ = scamstr($1); }
    | '[' expression_star ']' { $$ = $2; $$->type = SCAM_LIST; }
    | '{' dictionary_list '}' { $$ = $2; scamseq_prepend($$, scamsym("dict")); }
    ;
dictionary_list:
    dictionary_list dictionary_item { $$ = $1; scamseq_append($$, $2); }
    | { $$ = scamsexpr(); }
    ;
dictionary_item:
    expression ':' expression { $$ = scamsexpr_from_vals(2, $1, $3); }
    ;
%%

void yyerror(const char* s) {
    puts(s);
}

int main(int argc, char* argv[]) {
    int ret = yyparse();
    if (root) {
        scamval_println(root);
    } else {
        puts("Error: root is NULL");
    }
    return 0;
}
