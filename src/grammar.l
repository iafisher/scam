%option outfile="flex.c" header-file="flex.h"
%option reentrant bison-bridge
%option noyywrap nounput noinput

%{
#include <string.h>
#include "grammar.h"

int yylex();
%}

SYMBOL  [a-zA-Z=/*+^%!?<>-][a-zA-Z0-9=/*+^%!?<>-]*

%%
define { return DEFINE; }
true { return TRUE; }
false { return FALSE; }
-?[0-9]+ { yylval->ival = strtoll(yytext, NULL, 10); return INT; }
-?[0-9]+\.[0-9]+ { yylval->fval = strtod(yytext, NULL); return FLOAT; }
{SYMBOL} { yylval->sval = strdup(yytext); return SYMBOL; }
{SYMBOL}(\.{SYMBOL})+ { yylval->sval = strdup(yytext); return DOTTED_SYMBOL; }
\"(\\.|[^\\"])*\" { yylval->sval = strdup(yytext); return STRING; }
[^ \t\r\n] { return yytext[0]; }
;[^\n]*$ ;
.|\n ;
%%
