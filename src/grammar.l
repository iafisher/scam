%option outfile="flex.c" header-file="flex.h"
%option reentrant bison-bridge
%option noyywrap nounput noinput

%{
#include <string.h>
#include "grammar.h"

int yylex();

char* strdup_no_quotes(const char* s) {
    char* ret = strdup(s + 1);
    size_t n = strlen(ret);
    ret[n - 1] = '\0';
    return ret;
}
%}

%%
define { return DEFINE; }
-?[0-9]+ { yylval->ival = strtoll(yytext, NULL, 10); return INT; }
-?[0-9]+\.[0-9]+ { yylval->fval = strtod(yytext, NULL); return FLOAT; }
[a-zA-Z=/*+^%!?<>-][a-zA-Z0-9=/*+^%!?<>-]* { yylval->sval = strdup(yytext); return SYMBOL; }
\"(\\.|[^\\"])*\" { yylval->sval = strdup_no_quotes(yytext); return STRING; }
[^ \t\r\n] { return yytext[0]; }
. ;
%%
