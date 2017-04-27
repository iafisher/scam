%{
#include <stdio.h>
#include "scam.tab.h"

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
[a-zA-Z=/*+^%!?<>-][a-zA-Z0-9=/*+^%!?<>-]* { yylval.sval = strdup(yytext); return SYMBOL; }
-?[0-9]+ { yylval.ival = strtoll(yytext, NULL, 10); return INT; }
-?[0-9]+\.[0-9]+ { yylval.fval = strtod(yytext, NULL); return FLOAT; }
\"(\\.|[^\\"])*\" { yylval.sval = strdup_no_quotes(yytext); return STRING; }
[^ \t\r\n] { return yytext[0]; }
. ;
%%
