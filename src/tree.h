/***************************************************
* Tree or DAG related functions for Mila compiler  *
***************************************************/
#ifndef TREE_H_
#define TREE_H_ 1

#include <common.h>
#include "symtab.h"
#include "3ac.h"

BEGIN_C_DECLS

/* Bison generates own integer values
 * for tokens
 */
typedef int token_type;

typedef enum
{
    CL_STATEMENT,
    CL_EXPRESSION
} node_class;

typedef enum
{
    UNKNOWN_T = 0,
    IF_T,
    WHILE_T,
    ASSIGN_T,
    READ_T,
    WRITE_T,
    ID_T,
    CONST_T,
    BIOP_T,
    UOP_T,
} node_type;

typedef struct node_flag_s
{
    int visited : 1;
    int leaf : 1;
    int unused : 6;
} node_flag_t;

#define MAXCHILDREN 3
#define MAXSUBIDS 1024

typedef struct tree_node_s
{
    struct tree_node_s * child[MAXCHILDREN];
    struct tree_node_s * sibling;
    uint32_t id;
    int32_t lineno;
    node_class klass;
    node_type type;
    node_flag_t flags;

    union
    {
        token_type op;
        symbol_t * symbol;
    } attr;

    tac_t tac;
    
} tree_node_t;

int new_nodes(uint32_t size);
    
tree_node_t * new_write(tree_node_t *l);

tree_node_t * new_assign(tree_node_t *l, symbol_t *s);

tree_node_t * new_read(symbol_t *s);

tree_node_t * new_id(symbol_t *s);

tree_node_t * new_const(int32_t val);

tree_node_t * new_binop(tree_node_t *l, tree_node_t *r, token_type op);

tree_node_t * new_unop(tree_node_t *l, token_type op);

tree_node_t * new_if(tree_node_t *c, tree_node_t *t, tree_node_t *e);

tree_node_t * new_loop(tree_node_t *c, tree_node_t *b);

END_C_DECLS
    
#endif /* TREE_H_ */

