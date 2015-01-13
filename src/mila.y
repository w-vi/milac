/*****************************************************
 * bison/yacc specification for Mila language        *
 *****************************************************/
%{
#include "global.h"
#include "util.h"

#define YYERROR_VERBOSE 1 
#define YYDEBUG 0 

/* get rid of the implicit declaration warning*/
extern int yylex(void);
int yyerror(void** synatx_tree, const char* message);
%}

%parse-param {void** syntax_tree}
         
%token VAR CONST BGN ENDB IF THEN ELSE  WHILE DO READ WRITE
%token ID NUM 
%token EQ NEQ LT GT LTE GTE LPAR RPAR ASSGN COMMA SEMI PLUS MINUS TIMES DIVIDE

%right THEN ELSE

%% /* Grammar for MILA */

program     : decls block { *syntax_tree = (void *)$2.n;} 
            ;

decls       : constdecl decls {}
            | vardecl decls {}
            |
            ;

constdecl   : CONST ID EQ NUM const_list SEMI
                {
                    symbol_t *s = st_insert($2.s, lineno, TRUE);
                    s->type = SCONST;
                    s->node = new_const($4.v);
                    s->val = $4.v;
                    free($2.s);
                }
            ;

vardecl     : VAR ID var_list SEMI
                {
                    symbol_t *s = st_insert($2.s, lineno, TRUE);
                    s->type = SVAR;
                    s->node = new_id(s);
                    free($2.s);
                }
            ;

const_list  : COMMA ID EQ NUM const_list
                {
                    symbol_t *s = st_insert($2.s, lineno, TRUE);
                    s->type = SCONST;
                    s->node = new_const($4.v);
                    s->val = $4.v;
                    free($2.s);
                }
            |   {}        
            ;

var_list    : COMMA ID var_list
                {
                    symbol_t *s = st_insert($2.s, lineno, TRUE);
                    s->type = SVAR;
                    s->node = new_id(s);
                    free($2.s);
                }
            |   {}        
            ;

block       : BGN statm_seq SEMI ENDB {$$ = $2;}

statm_seq   : statm_seq SEMI statm 
                 { tree_node_t *t = $1.n;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                       {
                        t = t->sibling;
                       }
                     t->sibling = $3.n;
                     $$ = $1; }
                   else
                   {
                       $$ = $3;
                   }
                 }
            | statm {$$ = $1;}
            ;

statm       : if_stmt {$$ = $1;}
            | while_stmt {$$ = $1;}
            | assign_stmt {$$ = $1;}
            | write_stmt {$$ = $1;}
            | read_stmt {$$ = $1;}
            | block {$$ = $1;}
            ;

if_stmt     : IF cond THEN statm 
                 { $$.n = new_if($2.n, $4.n, NULL);}
            | IF cond THEN statm ELSE statm
                 { $$.n = new_if($2.n, $4.n, $6.n);}
            ;
while_stmt  : WHILE cond DO statm
                { $$.n = new_loop($2.n, $4.n); }
            ;
assign_stmt : ID ASSGN exp 
                 {
                     symbol_t *s = st_lookup($1.s);
                     if(s != NULL && s->type != SCONST)
                     {
                         $$.n = new_assign($3.n, s);
                     }
                     else
                     {
                         char err[256];
                         snprintf(err, 256, "Undeclared variable %s", $1.s);
                         yyerror(syntax_tree, err);
                     }
                 }
            ;
read_stmt   : READ ID
                 {
                     symbol_t *s = st_lookup($2.s);
                     if(s != NULL && s->type != SCONST)
                     {
                         $$.n = new_read(s);
                     }
                     else
                     {
                         char err[256];
                         snprintf(err, 256, "Undeclared variable %s", s->name);
                         yyerror(syntax_tree, err);
                     }
                 }
            ;
write_stmt  : WRITE exp { $$.n = new_write($2.n); }
            ;
cond        : exp LT exp { $$.n = new_binop($1.n, $3.n, LT);}
            | exp GT exp { $$.n = new_binop($1.n, $3.n, GT);}
            | exp EQ exp { $$.n = new_binop($1.n, $3.n, EQ);}
            | exp NEQ exp { $$.n = new_binop($1.n, $3.n, NEQ);}
            | exp LTE exp { $$.n = new_binop($1.n, $3.n, LTE);}
            | exp GTE exp { $$.n = new_binop($1.n, $3.n, GTE);}
            | exp { $$ = $1; }
            ;
exp         : exp PLUS term  { $$.n = new_binop($1.n, $3.n, PLUS);}
            | exp MINUS term { $$.n = new_binop($1.n, $3.n, MINUS);}
            | MINUS term {$$.n = new_unop($2.n, MINUS);}
            | term { $$ = $1; }
            ;

term        : term TIMES factor { $$.n = new_binop($1.n, $3.n, TIMES);}
            | term DIVIDE factor { $$.n = new_binop($1.n, $3.n, DIVIDE);}
            | factor { $$ = $1; }
            ;
factor      : LPAR exp RPAR { $$ = $2; }
            | NUM { $$.n = new_const($1.v);}
            | ID {
                   symbol_t *s = st_lookup($1.s);
                   if(NULL == s)
                   {
                       char err[256];
                       snprintf(err, 256, "Undeclared identifier %s", s->name);
                       yyerror(syntax_tree, err);
                   }
                   else
                   {
                       $$.n = s->node;
                   }
                 }
            ;
%%

int yyerror(void **synatx_tree, const char *message)
{
    fprintf(out, "Syntax error at line %d: %s\n", lineno, message);
    return 1;
}
