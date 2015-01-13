/*****************************************************
 * flex specification for Mila language              *
 *****************************************************/

%{
#include "global.h"
#include "util.h"
#include "mila.tab.h"

extern YYSTYPE yylval; 

void print_error(void);
extern uint32_t lineno;

%}

%option nounput
%option noyywrap
 /*%option debug*/

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     (\r\n)|(\n)
whitespace  [ \t\b\v\f]+

%%

"var"           {return VAR; }
"const"         {return CONST; }
"begin"         {return BGN; }
"end"           {return ENDB; }
"if"            {return IF; }
"then"          {return THEN; }
"else"          {return ELSE; }
"while"         {return WHILE; }
"do"            {return DO; }
"read"          {return READ; }
"write"         {return WRITE; }
"="             {return EQ; }
"<>"            {return NEQ; }
"<"             {return LT; }
">"             {return GT; }
"=<"            {return LTE; }
">="            {return GTE; }
"("             {return LPAR; }
")"             {return RPAR; }
":="            {return ASSGN; }
","             {return COMMA; }
";"             {return SEMI; }
"+"             {return PLUS; }
"-"             {return MINUS; }
"*"             {return TIMES; }
"/"             {return DIVIDE; }
{newline}       {++lineno;}
{number}        {yylval.v = atoi(yytext); return NUM;}
{identifier}    {yylval.s = strdup(yytext); return ID;}
{whitespace}    {/* skip whitespace */}
"{"             { char c;
                  do
                  { c = input();
                    if (c == EOF) break;
                    if (c == '\n') ++lineno;
                  } while (c != '}');
                }

.               {print_error();}

%%


void print_error(void)
{
    fprintf(stderr, "Syntax error : Unknown symbol %s on line %d\n", yytext, lineno);
}
