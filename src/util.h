/***************************************************
* Utility functions for Mila compiler              *
***************************************************/
#ifndef UTIL_H_
#define UTIL_H_ 1

#include <common.h>
#include "tree.h"
#include "3ac.h"

BEGIN_C_DECLS

void reset_visited(void * n);

void print_token(token_type tok, const char *token_str, FILE *out);

void print_tree(tree_node_t *tree);

void print_node(tree_node_t *node);

void  print_graph(FILE *f, char *name, tree_node_t *tree, hasht_tab_t *nodes_tab);

int strrpl(const char *str, const char *substr, const char *repl);

END_C_DECLS
    
#endif /* UTIL_H_ */

