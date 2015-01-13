/***************************************************
* Common Subexpression elimination Mila compile    *
***************************************************/
#include "global.h"
#include "util.h"
#include "optimize.h"
#include "bitset.h"

#define CSE_TAB_SIZE 512

typedef struct node_s
{
    uint32_t id;
    int generated;
    tac_t *ins;
    codeop_type op;
    struct node_s *l;
    struct node_s *r;
    symbol_t *s;
    list(symbol_t *, symbols);
} node_t;

static uint32_t node_count = 0;

static bb_t *cur_bb = NULL;

#define NEW_NODE(p)do{                                  \
if(NULL == (p = (node_t*)calloc(1, sizeof(node_t)))) {  \
    fprintf(stderr, "Memory allocation error.\n");      \
    exit(EXIT_FAILURE);                                 \
}                                                       \
memset(&p->symbols, 0, sizeof(p->symbols));              \
p->id = ++node_count;                                   \
p->op = TNONE;                                          \
list_push(nodes, p);                                    \
} while(0)                                              \


static hasht_tab_t * nodes_tab = NULL;
static list(symbol_t *, symbols);
list(tac_t *, newcode);
static list(node_t *, nodes);

char *
nodeop(char *str, codeop_type op, uint32_t x)
{
    switch(op)
    {
    case TIN:
        sprintf(str, "r%d", x);
        break;
    case TOUT:
        sprintf(str, "w%d", x);
        break;
    case TIFF:
        sprintf(str, "if%d", x);
        break;
    default:
        ASSERT(0);
    }

    return str;
}

char *
binop(char *str, codeop_type op, uint32_t x, uint32_t y)
{
    uint32_t t = x;
    
    if((op == TADD || op == TMUL) && x > y)
    {
        x = y;
        y = t;
    }
    
    switch(op)
    {
    case TEQ:
        sprintf(str, "=%d%d", x, y);
        break;
    case TNEQ:
        sprintf(str, "<>%d%d", x, y);
        break;
    case TLT:
        sprintf(str, "<%d%d", x, y);
        break;
    case TGT:
        sprintf(str, ">%d%d", x, y);
        break;
    case TLE:
        sprintf(str, "=<%d%d", x, y);
        break;
    case TGE:
        sprintf(str, ">=%d%d", x, y);
        break;
    case TADD:
        sprintf(str, "+%d%d", x, y);
        break;
    case TSUB:
        sprintf(str, "-%d%d", x, y);
        break;
    case TMUL:
        sprintf(str, "*%d%d", x, y);
        break;
    case TDIV:
        sprintf(str, "/%d%d", x, y);
        break;
    default:
        ASSERT(0);
    }

    return str;
}


void
p_node(node_t *n)
{
    char buff[1024];

    fprintf(out, "%d\t", n->id);

    if(n->op != TNONE)
    {
        binop(buff, n->op, n->l->id, n->r->id);
        fprintf(out, "%s\t", buff);
    }
    else
    {
        fprintf(out, "none\t");
    }
    

    list_each(n->symbols, s)
    {
        fprintf(out, "%s ", s->name);
    }
    
    fprintf(out, "\n");
}

void
pp_node(void *n)
{
    p_node((node_t *)n);
}

void
dump_tab()
{
    fprintf(out, "ID\t OP\t Symbols\n");
    hash_tab_iterate(nodes_tab, pp_node);
}

void
skip_free(void *arg)
{
}

int
cse_init(bb_t *b)
{
    if(NULL != nodes_tab)
    {
        return FALSE;
    }

    if((nodes_tab = hash_tab_init(nodes_tab, CSE_TAB_SIZE, NULL, skip_free)) == NULL)
    {
        return FALSE;
    }

    memset(&symbols, 0, sizeof(symbols));

    memset(&nodes, 0, sizeof(nodes));

    cur_bb = b;
    
    return TRUE;
}

void
cse_delete()
{
    list_clear(symbols);

    newcode = NULL;

    list_each(nodes, n)
    {
         list_clear(n->symbols);
         free(n);
    }
    list_clear(nodes);

    if(NULL != nodes_tab)
    {
        hash_tab_destroy(nodes_tab);
        nodes_tab = NULL;
    }

    cur_bb = NULL;
}

void
symbol_list(void *symb)
{
    if(((symbol_t *)symb)->type != SLABEL)
        list_push(symbols, (symbol_t *)symb);
}

static int
gen_node(node_t *n, symbol_t *s , symbol_t **gen_s)
{
    tac_t *j = NULL;
    tac_t *k = NULL;
    symbol_t *gs1 = NULL;
    symbol_t *gs2 = NULL;

    j = n->ins;

    TRACE(15, "gen for %s", s->name);
//   if(j != NULL) print_tac(j, 777);

    if(n->op == TNONE)
    {
        TRACE(15, "op none%s", "");
        *gen_s = s;
        return 0;
    }
    
    if(n->generated)
    {
        if(list_length(n->symbols))
            *gen_s = list_front(n->symbols);
        else
            *gen_s = n->s;

        TRACE(15, "Already done %s = %s", s->name, (*gen_s)->name);

        return 0;
    }
    
    if(n->l->op != TNONE) gen_node(n->l, j->a1, &gs1);
    if(n->r->op != TNONE) gen_node(n->r, j->a2, &gs2);


    NEW_TAC(k);
    k->op = n->op;
    if(list_length(n->l->symbols))
        k->a1 = list_front(n->l->symbols);
    else
        k->a1 = n->l->s;

    if(list_length(n->r->symbols))
        k->a2 = list_front(n->r->symbols);
    else
        k->a2 = n->r->s
            ;
    if(list_length(n->symbols))
        k->r = list_front(n->symbols);
    else
        k->r = n->s;
//    k->r = s;
    k->bb = cur_bb;
    list_push(newcode, k);
    n->generated = TRUE;
    *gen_s = k->r;

    TRACE(15, "generated for %s = %s", s->name, (*gen_s)->name);
//    print_tac(k, 555);

    return 1;
}


static void
gen(bb_t *bb)
{
    node_t *n = NULL;
    tac_t *t = NULL;
    symbol_t *gen = NULL;
    
    memset(&newcode, 0, sizeof(newcode));

    list_each(bb->ins, ins)
    {
        if(ins->op == TIN || ins->op == TLABEL)
        {
//            print_tac(ins, 999);
   
            NEW_TAC(t);
            memcpy(t, ins, sizeof(tac_t));
            list_push(newcode, t);
            continue;
        }

        if(ins->op == TOUT)
        {
//            print_tac(ins, 888);
            n = hash_tab_lookup(nodes_tab, ins->r->name);
            if(NULL != n)
            {
#ifdef DEBUG               
                 symbol_t *x = list_front(n->symbols);
#endif                 
                TRACE(15, "OUT not null %s front %s", ins->r->name, x->name);
                gen_node(n, ins->r, &gen);
                ins->r = gen;
                TRACE(15, "OUT after %s", ins->r->name);
//                print_tac(ins, 888888);
            }
            NEW_TAC(t);
            memcpy(t, ins, sizeof(tac_t));
            list_push(newcode, t);
            continue;
        }

        if(ins->op == TIFF || ins->op == TJMP)
        {
            list_each(symbols, s)
            {
                if(BITTEST(bb->lva.out, s->id)) //is live
                {
                    TRACE(15, "name %s", s->name);
                    
                    n = hash_tab_lookup(nodes_tab, s->name);
                    if(NULL != n) gen_node(n, s, &gen);
                }
            }
            if(ins->op == TIFF)
            {
//                print_tac(ins, 999);
                n = hash_tab_lookup(nodes_tab, ins->a1->name);
                ASSERT(NULL != n);
                if(! gen_node(n, ins->a1, &gen))
                    ins->a1 = gen;
                NEW_TAC(t);
                memcpy(t, ins, sizeof(tac_t));
                list_push(newcode, t);
            }
            else
            {
                NEW_TAC(t);
                memcpy(t, ins, sizeof(tac_t));
                list_push(newcode, t);
            }
            continue;
        }

        if(ins->op == TASSIGN)
        {
//            print_tac(ins, 111);
            n = hash_tab_lookup(nodes_tab, ins->r->name);
            ASSERT(NULL != n);
            if(!gen_node(n, ins->r, &gen))
            {
                ASSERT(gen);
                ins->r = gen;
//                print_tac(ins, 111111);
                NEW_TAC(t);
                memcpy(t, ins, sizeof(tac_t));
                list_push(newcode, t);
            }
            
            continue;
        }
    }

    list_each(symbols, s)
    {
        if(BITTEST(bb->lva.out, s->id)) //is live
        {
            TRACE(15, "name %s", s->name);
            
            n = hash_tab_lookup(nodes_tab, s->name);
            if(NULL != n) gen_node(n, s, &gen);
        }
    }
    
    list_each(bb->ins, ins)
    {
        tac_free(ins);
    }

    list_clear(bb->ins);
    
    bb->ins = newcode;
}

bb_t *
cse(bb_t *bb)
{
    char buff[1024];
    node_t *a = NULL;
    node_t *b = NULL;
    node_t *n = NULL;

    TRACE(5, "Common Subexpression elimination pass on bb %d", bb->bbid);

    if(!cse_init(bb))
    {
        return NULL;
    }

    list_each(bb->ins, ins)
    {
        if(tac_is_binop(ins->op))
        {
            a = hash_tab_lookup(nodes_tab, ins->a1->name);
            if(NULL ==  a)
            {
                NEW_NODE(a);
                a->s = ins->a1;
                TRACE(15, "new node %s %d", ins->a1->name, a->id);
                hash_tab_insert(nodes_tab, ins->a1->name, a);
                list_push(a->symbols, ins->a1);
            }

            b = hash_tab_lookup(nodes_tab, ins->a2->name);
            if(NULL ==  b)
            {
                NEW_NODE(b);
                b->s = ins->a2;
                TRACE(15, "new node %s %d", ins->a2->name, b->id);
                hash_tab_insert(nodes_tab, ins->a2->name, b);
                list_push(b->symbols, ins->a2);
            }
            
            binop(buff, ins->op, a->id, b->id);
            TRACE(15, "binop : %s", buff);
            n = hash_tab_lookup(nodes_tab, buff);
            if(NULL ==  n)
            {
                NEW_NODE(n);
                TRACE(15, "new node %s %d", buff, n->id);
                n->op = ins->op;
                n->l = a;
                n->r = b;
                hash_tab_insert(nodes_tab, buff, n);
            }
            n->ins = ins;
            list_push(n->symbols, ins->r);
            a = hash_tab_lookup(nodes_tab, ins->r->name);
            if(NULL != a)
            {
                list_remove(a->symbols, ins->r);
                hash_tab_update(nodes_tab, ins->r->name, a);
            }
            if(ins->r == ins->a1 || ins->r == ins->a2)
            {
                hash_tab_update(nodes_tab, ins->r->name, n);
            }
            else
            {
                hash_tab_insert(nodes_tab, ins->r->name, n);
            }
            continue;
        }

        if(ins->op == TASSIGN)
        {
            a = hash_tab_lookup(nodes_tab, ins->a1->name);
            if(NULL ==  a)
            {
                TRACE(15, "new node %s", ins->a1->name);

                NEW_NODE(a);
                hash_tab_insert(nodes_tab, ins->a1->name, a);
                list_push(a->symbols, ins->a1);
            }
            list_push(a->symbols, ins->r);
            n = hash_tab_lookup(nodes_tab, ins->r->name);
            if(NULL != n)
            {
                list_remove(n->symbols, ins->r);
                hash_tab_update(nodes_tab, ins->r->name, a);
            }
            a->ins = ins;
            hash_tab_insert(nodes_tab, ins->r->name, a);
            continue;
        }
    }

    

//    hash_tab_dump(nodes_tab, out, 0);
//    dump_tab();
    
    st_iterate(symbol_list);

    gen(bb);

    cse_delete();
    
    return bb;
}


