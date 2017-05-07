%option outfile="flex.c" header-file="flex.h"
%option reentrant bison-bridge
%option noyywrap nounput noinput

%{
#include <string.h>
#include "grammar.h"

int yylex();
%}

%%
define { return DEFINE; }
true { return TRUE; }
false { return FALSE; }
-?(0|[1-9][0-9]*) { yylval->ival = strtoll(yytext, NULL, 10); return INT; }
-?0x[0-9A-Fa-f]+ { yylval->ival = strtoll(yytext, NULL, 16); return INT; }
-?[0-9]+\.[0-9]+ { yylval->fval = strtod(yytext, NULL); return FLOAT; }
([a-zA-Z=/*+^%!?<>_][a-zA-Z0-9=/*+^%!?<>_-]*|-) { yylval->sval = strdup(yytext); return SYMBOL; }
\"(\\.|[^\\"])*\" { yylval->sval = strdup(yytext); return STRING; }
[^ \t\r\n] { return yytext[0]; }
;[^\n]*$ ;
.|\n ;
%%
