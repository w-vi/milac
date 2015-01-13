/***************************************************
* Global types and vars for Mila compiler          *
****************************************************/
#ifndef GLOBAL_H_
#define GLOBAL_H_ 1

#include <common.h>
#include "symtab.h"
#include "tree.h"
#include "hashtab.h"

BEGIN_C_DECLS

extern FILE* src_file; /* source code file */
extern FILE* out; /* listing output file */
extern FILE* tm_file; /* tm code output file */

extern uint32_t lineno; /* source line number for listing */

#define SYMBOL_TAB_SIZE 4086
#define NODE_TAB_SIZE 2048

typedef union  yylval_u
{
    tree_node_t * n;
    char * s;
    int v;
} myylval_t;

#define YYSTYPE myylval_t
#define YYSTYPE_IS_DECLARED 1

/*
  Tracing flags ... useful for debugging
*/

extern int8_t trace_parse;
extern int8_t trace_analyze;
extern int8_t trace_code;


END_C_DECLS
    
#endif /* GLOBAL_H_ */

