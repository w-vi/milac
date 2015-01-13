/***************************************************
* Tree or DAG  function for Mila compiler          *
***************************************************/
#include "global.h"
#include "util.h"
#include "tree.h"
#include "mila.tab.h"

char* nodestr[] = {"UNKNOWN", "IF", "WHILE", "ASSIGN", "READ", "WRITE", "ID", "CONST", "BIOP", "UOP"};

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static uint32_t nodeid = 0;
hasht_tab_t * nodes_tab = NULL;

extern uint32_t lineno;

int
new_nodes(uint32_t size)
{
    if(NULL != nodes_tab)
    {
        return 0;
    }

    if((nodes_tab = hash_tab_init(nodes_tab, size, NULL, &free)) == NULL)
    {
        return -1;
    }

    return 0;
}

tree_node_t *
new_node()
{
    int32_t i = 0;
    tree_node_t * t = NULL;
    
    if(NULL == (t = (tree_node_t *)calloc(1, sizeof(tree_node_t))))
    {
        fprintf(out, "Out of memory error at line %d\n", lineno);
    }
    else
    {
        for(i = 0; i < MAXCHILDREN; ++i)
        {
            t->child[i] = NULL;
        }
        t->id = ++nodeid;
        t->sibling = NULL;
        t->klass = CL_STATEMENT;
        t->type = UNKNOWN_T;
        t->lineno = lineno;
        t->flags.visited = FALSE;
        t->flags.leaf = FALSE;
    }
    return t;
}

char *
node_str(char *str, node_type type, uint32_t x, uint32_t y,  uint32_t z)
{
    if(0 == z)
    {
        if(0 == y)
        {
            sprintf(str, "%s%d", nodestr[type], x);
        }
        else
        {
            sprintf(str, "%s%d%d", nodestr[type], x, y);
        }
    }
    else
    {
        sprintf(str, "%s%d%d%d", nodestr[type], x,y,z);
    }
    return str;
}

/* char * */
/* binop_str(char *str, token_type op, tree_node_t *x, tree_node_t *y) */
/* { */
/*     tree_node_t *t = x; */
    
/*     if((op == PLUS || op == TIMES) && x > y) */
/*     { */
/*         x = y; */
/*         y = t; */
/*     } */
    
/*     switch(op) */
/*     { */
/*     case EQ: */
/*         sprintf(str, "=%d%d", x->id, y->id); */
/*         break; */
/*     case NEQ: */
/*         sprintf(str, "<>%d%d", x->id, y->id); */
/*         break; */
/*     case LT: */
/*         sprintf(str, "<%d%d", x->id, y->id); */
/*         break; */
/*     case GT: */
/*         sprintf(str, ">%d%d", x->id, y->id); */
/*         break; */
/*     case LTE: */
/*         sprintf(str, "=<%d%d", x->id, y->id); */
/*         break; */
/*     case GTE: */
/*         sprintf(str, ">=%d%d", x->id, y->id); */
/*         break; */
/*     case PLUS: */
/*         sprintf(str, "+%d%d", x->id, y->id); */
/*         break; */
/*     case MINUS: */
/*         sprintf(str, "-%d%d", x->id, y->id); */
/*         break; */
/*     case TIMES: */
/*         sprintf(str, "*%d%d", x->id, y->id); */
/*         break; */
/*     case DIVIDE: */
/*         sprintf(str, "/%d%d", x->id, y->id); */
/*         break; */
/*     default: */
/*         sprintf(str, "?%d%d", x->id, y->id); */
/*     } */

/*     return str; */
/* } */

/* char * */
/* multiop_str(char *str, token_type op, uint32_t *ids) */
/* { */

/*     char *p = str; */
/*     switch(op) */
/*     { */
/*     case PLUS: */
/*         p += sprintf(str, "+%d", *ids); */
/*         break; */
/*     case MINUS: */
/*         p += sprintf(str, "-%d", *ids); */
/*         break; */
/*     case TIMES: */
/*         p += sprintf(str, "*%d", *ids); */
/*         break; */
/*     case DIVIDE: */
/*         p += sprintf(str, "/%d", *ids); */
/*         break; */
/*     default: */
/*         p += sprintf(str, "?%d", *ids); */
/*     } */

/*     ++ids; */
    
/*     while(*ids != 0) */
/*     { */
/*         p += sprintf(p, "%d", *ids); */
/*         ++ids; */
/*     } */

/*     return str; */
/* } */

tree_node_t *
new_write(tree_node_t *l)
{
    tree_node_t *t = NULL;
    char buff[128];
    
    if(NULL != (t = new_node()))
    {
        t->type = WRITE_T;
        t->child[0] = l;
        node_str(buff, WRITE_T, l->id, 0, 0);
        if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)t))
        {
            free(t);
            t = NULL;
        }
        t->tac.op = TOUT;
        t->tac.r = l->tac.r;
    }
    
    return t;
}

tree_node_t *
new_assign(tree_node_t *l, symbol_t *s)
{
    tree_node_t *t = NULL;
    char buff[128];

    ASSERT(NULL != l && NULL != s);

    if(NULL != (t = new_node()))
    {
        t->type = ASSIGN_T;
        t->child[0] = l;
        t->attr.symbol = s;
        node_str(buff, ASSIGN_T, l->id, s->node->id, 0);
        if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)t))
        {
            free(t);
            t = NULL;
        }
        t->tac.op = TASSIGN;
        t->tac.a1 = l->tac.r;
        t->tac.r = s;
    }

    return t;
}

tree_node_t *
new_read(symbol_t *s)
{
    tree_node_t *t = NULL;
    char buff[128];

    ASSERT(NULL != s);

    if(NULL != (t = new_node()))
    {
        t->type = READ_T;
        t->child[0] = NULL;
        t->attr.symbol = s;
        node_str(buff, READ_T, 0, s->node->id, 0);
        if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)t))
        {
            free(t);
            t = NULL;
        }
        t->tac.op = TIN;
        t->tac.r = s;
    }

    return t;
}

tree_node_t *
new_const(int32_t val)
{
    tree_node_t * t = NULL;
    char buff[128];
    
    t = hash_tab_lookup(nodes_tab, node_str(buff, CONST_T, val, 0, 0));
    if(NULL == t)
    {
        if(NULL != (t = new_node()))
        {
            t->klass = CL_EXPRESSION;           
            t->type = CONST_T;
            t->attr.symbol =  st_insert_const(val);
            t->flags.leaf = TRUE;
            if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)t))
            {
                free(t);
                t = NULL;
            }
            t->tac.r = t->attr.symbol;
            t->attr.symbol->node = t;
        }
    }

    return t;
}

tree_node_t *
new_id(symbol_t *s)
{
    tree_node_t * t = NULL;

    if(NULL == s->node)
    {
        if(NULL != (t = new_node()))
        {
            t->klass = CL_EXPRESSION;
            t->type = ID_T;
            t->flags.leaf = TRUE;
            t->attr.symbol = s;
            t->tac.r = s;
        }
        
        if(HASH_OK != hash_tab_insert(nodes_tab, s->name, (void *)t))
        {
            free(t);
            t = NULL;
        }
    }
    else
    {
        t = s->node;
    }

    return t;
}

tree_node_t *
const_fold(token_type op, tree_node_t *l, tree_node_t *r)
{
    int32_t res = 0;
    
    switch (op)
    {
    case PLUS:
        res = l->attr.symbol->val + r->attr.symbol->val;
        break;
    case MINUS:
        res = l->attr.symbol->val - r->attr.symbol->val;
        break;
    case TIMES:
        res = l->attr.symbol->val * r->attr.symbol->val;
        break;
    case DIVIDE:
        res = l->attr.symbol->val / r->attr.symbol->val;
        break;
    case EQ:
        res = l->attr.symbol->val == r->attr.symbol->val;
        break;
    case NEQ:
        res = l->attr.symbol->val != r->attr.symbol->val;
        break;
    case LT:
        res = l->attr.symbol->val < r->attr.symbol->val;
        break;
    case GT:
        res = l->attr.symbol->val > r->attr.symbol->val;
        break;
    case LTE:
        res = l->attr.symbol->val <= r->attr.symbol->val;
        break;
    case GTE:
        res = l->attr.symbol->val >= r->attr.symbol->val;
        break;
    default:
        break;
    }

    return new_const(res);
}

tree_node_t *
new_binop(tree_node_t *l, tree_node_t *r, token_type op)
{
    tree_node_t * t = NULL;
    char buff[1024];

    ASSERT(NULL != l);
    ASSERT(NULL != r);
    
    if(l->type == CONST_T && r->type == CONST_T)
    {
        return const_fold(op, l, r);
    }

    if(NULL == t)
    {
        if(NULL != (t = new_node()))
        {
            t->klass = CL_EXPRESSION;
            t->type = BIOP_T;
            t->child[0] = l;
            t->child[1] = r;
            t->attr.op = op;
            if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)t))
            {
                free(t);
                t = NULL;
            }

            tac_gen(t);
        }
    }

    return t;
}

tree_node_t *
new_unop(tree_node_t *t, token_type op)
{
    tree_node_t *n = NULL;
    char buff[128];
    
    if(t->type == CONST_T)
    {
        n = new_const(-t->attr.symbol->val);
    }
    else
    {
        n = hash_tab_lookup(nodes_tab, node_str(buff, UOP_T, t->id, 0, 0));
        if(NULL == n)
        {
            if(NULL != (n = new_node()))
            {
                n->klass = CL_EXPRESSION;
                n->type = UOP_T;
                n->child[0] = t;
                n->attr.op = op;
                if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)n))
                {
                    free(n);
                    n = NULL;
                }
                tac_gen(n);
            }
        }
    }

    return n;
}

tree_node_t *
new_if(tree_node_t *c, tree_node_t *t, tree_node_t *e)
{
    tree_node_t *n = NULL;
    char buff[128];
    
    if(c->type == CONST_T)
    {
        n =  c->attr.symbol->val > 0 ? t : e;
    }
    else
    {
        if(NULL != (n = new_node()))
        {
            ASSERT(c);
            ASSERT(t);
            n->type = IF_T;
            n->child[0] = c;
            n->child[1] = t;
            n->child[2] = e;
            if(e)
            {
                node_str(buff, IF_T, c->id, t->id, e->id);
            }
            else
            {
                node_str(buff, IF_T, c->id, t->id, 0);
            }
            
            if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)n))
            {
                free(n);
                n = NULL;
            }

            tac_gen(n);
        }
    }

    return n;
}

tree_node_t *
new_loop(tree_node_t *c, tree_node_t *b)
{
    tree_node_t *n = NULL;
    char buff[128];

    if(c->type == CONST_T)
    {
        if(c->attr.symbol->val > 0)
        {
            fprintf(out, "Infinite loop with no break line %d\n", lineno);
        }
    }
    else
    {
        if(NULL != (n = new_node()))
        {
            n->type = WHILE_T;
            n->child[0] = c;
            n->child[1] = b;
            node_str(buff, WHILE_T, c->id, b->id, 0);
            if(HASH_OK != hash_tab_insert(nodes_tab, buff, (void *)n))
            {
                free(n);
                n = NULL;
            }
        }
            
        tac_gen(n);

    }

    return n;
}

