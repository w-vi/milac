/***************************************************
* Utility function for Mila compiler               *
***************************************************/

#include "global.h"
#include "util.h"
#include "mila.tab.h"
#include "symtab.h"
#include "hashtab.h"

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int32_t indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

extern uint32_t lineno;

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void
print_token(token_type tok, const char* token_str, FILE *out)
{
    switch (tok)
    {
        case VAR:
        case CONST:
        case BGN:
        case ENDB:
        case IF:
        case THEN:
        case ELSE:
        case WHILE:
        case DO:
        case READ:
        case WRITE:
            fprintf(out, "keyword: %s ", token_str);
            break;
        case EQ: fprintf(out, "= ");
            break;
        case NEQ: fprintf(out, "<> ");
            break;
        case LT: fprintf(out, "< ");
            break;
        case GT: fprintf(out, "> ");
            break;
        case LTE: fprintf(out, "=< ");
            break;
        case GTE: fprintf(out, ">= ");
            break;
        case LPAR: fprintf(out, "( ");
            break;
        case RPAR: fprintf(out, ") ");
            break;
        case ASSGN: fprintf(out, ":= ");
            break;
        case COMMA: fprintf(out, "; ");
            break;
        case SEMI: fprintf(out, "; ");
            break;
        case PLUS: fprintf(out, "+ ");
            break;
        case MINUS: fprintf(out, "- ");
            break;
        case TIMES: fprintf(out, "* ");
            break;
        case DIVIDE: fprintf(out, "/ ");
            break;
        case NUM:
            fprintf(out, "NUM, val= %s ", token_str);
            break;
        case ID:
            fprintf(out, "ID, name= %s ", token_str);
            break;
        default: /* should never happen */
            fprintf(out, "Unknown token: %d ",tok);
    }
}

/* printSpaces indents by printing spaces */
static void
print_space(void)
{
    int32_t i;
    for(i = 0; i < indentno; ++i)
        fprintf(out, " ");
}

void
print_node(tree_node_t *node)
{
    if(!trace_parse)
        return;
    
    switch (node->type)
    {
    case IF_T:
        fprintf(out, "IF \n");
        break;
    case WHILE_T:
        fprintf(out, "WHILE \n");
        break;
    case ASSIGN_T:
        fprintf(out, "ASSIGN to: %s \n",node->attr.symbol->name);
        break;
    case READ_T:
        fprintf(out, "READ: %s \n",node->attr.symbol->name);
        break;
    case WRITE_T:
        fprintf(out, "WRITE \n");
        break;
    case BIOP_T:
        fprintf(out, "BINOP: ");
        print_token(node->attr.op,"\0", out);
        fprintf(out, "\n");
        break;
    case CONST_T:
        fprintf(out, "CONST: %d \n",node->attr.symbol->val);
        break;
    case ID_T:
        fprintf(out, "ID: %s \n",node->attr.symbol->name);
        break;
    case UOP_T:
        fprintf(out, "UNOP: %s \n",node->attr.symbol->name);
        break;
    default:
        fprintf(out, "Unknown Expression Node \n");
        break;
    }
}
    
/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void
print_tree(tree_node_t *tree)
{
    int32_t i = 0;
    INDENT;
    while(tree != NULL && !tree->flags.visited)
    {
        print_space();
        switch (tree->type)
        {
        case IF_T:
            fprintf(out, "IF\n");
            break;
        case WHILE_T:
            fprintf(out, "WHILE\n");
            break;
        case ASSIGN_T:
            fprintf(out, "ASSIGN to: %s\n",tree->attr.symbol->name);
            break;
        case READ_T:
            fprintf(out, "READ: %s\n",tree->attr.symbol->name);
            break;
        case WRITE_T:
            fprintf(out, "WRITE\n");
            break;
        case BIOP_T:
            fprintf(out, "BINOP: ");
            print_token(tree->attr.op,"\0", out);
            fprintf(out, "\n");
            break;
        case CONST_T:
            fprintf(out, "CONST: %d\n",tree->attr.symbol->val);
            break;
        case ID_T:
            fprintf(out, "ID: %s\n",tree->attr.symbol->name);
            break;
        case UOP_T:
            fprintf(out, "UNOP: -\n");
            break;
        default:
            fprintf(out, "Unknown Node\n");
            break;
        }
        
        for(i = 0; i < MAXCHILDREN; ++i)
            print_tree(tree->child[i]);
        tree = tree->sibling;
    }
    UNINDENT;
}


void
dot_node(void *n)
{
    tree_node_t *node = (tree_node_t*)n;
    switch (node->type)
    {
    case IF_T:
        fprintf(out, "%d[label=\"if(%d)\"];\n", node->id, node->lineno);
        break;
    case WHILE_T:
        fprintf(out, "%d[label=\"while(%d)\"];\n", node->id, node->lineno);
        break;
    case ASSIGN_T:
        fprintf(out, "%d[label=\"%s := \"];\n",node->id, node->attr.symbol->name);
        break;
    case READ_T:
        fprintf(out, "%d[label=\"read %s\"];\n",node->id, node->attr.symbol->name);
        break;
    case WRITE_T:
        fprintf(out, "%d[label=\"write %d\"];\n", node->id, node->lineno);
        break;
    case BIOP_T:
        switch (node->attr.op)
        {
        case PLUS:
            fprintf(out, "%d[label=\"+\"];\n", node->id);
            break;
        case MINUS:
            fprintf(out, "%d[label=\"-\"];\n", node->id);
            break;
        case TIMES:
            fprintf(out, "%d[label=\"*\"];\n", node->id);
            break;
        case DIVIDE:
            fprintf(out, "%d[label=\"/\"];\n", node->id);
            break;
        case EQ:
            fprintf(out, "%d[label=\"=\"];\n", node->id);
            break;
        case NEQ:
            fprintf(out, "%d[label=\"!=\"];\n", node->id);
            break;
        case LT:
            fprintf(out, "%d[label=\"<\"];\n", node->id);
            break;
        case GT:
            fprintf(out, "%d[label=\">\"];\n", node->id);
            break;
        case LTE:
            fprintf(out, "e%d[label=\"=<\"];\n", node->id);
            break;
        case GTE:
            fprintf(out, "e%d[label=\">=\"];\n", node->id);
            break;
        default:
            break;
        }
        break;
    case CONST_T:
        fprintf(out, "%d[label=\"const %d\"];\n",node->id, node->attr.symbol->val);
        break;
    case ID_T:
        fprintf(out, "%d[label=\"%s\"];\n", node->id, node->attr.symbol->name);
        break;
    case UOP_T:
        fprintf(out, "%d[label=\"-\"];\n", node->id);
        break;
    default:
        fprintf(out, "Unknown Node;\n");
        break;
    }
}
    
void
graph_nodes(tree_node_t *tree)
{
    int32_t i = 0;

    while(tree != NULL && !tree->flags.visited)
    {
        tree->flags.visited  = TRUE;
        if(tree->child[0] != NULL)
        {
            for(i = 0; i < MAXCHILDREN; ++i)
            {
                if(tree->child[i] != NULL)
                {
                    fprintf(out, "%d -> %d \n", tree->id, tree->child[i]->id);
                }
            }
        }
        for(i = 0; i < MAXCHILDREN; ++i)
            graph_nodes(tree->child[i]);
        
        tree = tree->sibling;
    }
}

void
reset_visited(void * n)
{
    ((tree_node_t*)n)->flags.visited = FALSE;
}

void
print_graph(FILE *f, char *name, tree_node_t *tree, hasht_tab_t *nodes_tab)
{
    FILE * o = out;
    out = f;
    hash_tab_iterate(nodes_tab, &reset_visited);
    fprintf(out, "digraph %s {\n", name);
    hash_tab_iterate(nodes_tab, &dot_node);
    graph_nodes(tree);
    fprintf(out, "}");
    out = o;
}

int
strrpl(const char *str, const char *substr, const char *repl)
{
    int i = 0;
    char *chr = NULL;
    size_t sub_len = 0;
    size_t repl_len = 0;
 
    ASSERT(str != NULL);
    ASSERT(substr != NULL);
    ASSERT(repl != NULL);
        
    chr = strstr(str, substr);
    if(chr == NULL) return 0;
 
    sub_len = strlen(substr);
    repl_len = strlen(repl);
 
    for(i = 0; i < (int)sub_len && i < (int)repl_len; ++i)
    {
        *chr = repl[i];
        chr++;
        if(chr == '\0') break;
    }
 
    return i;
}
