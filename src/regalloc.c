/***************************************************
* Register allocator for Mila compiler             *
***************************************************/
#include "global.h"
#include "util.h"
#include "gen.h"
#include "code.h"
#include "optimize.h"
#include "bitset.h"
#include "queue.h"
#include "stack.h"
                                       

typedef struct intp_s intp_t;

struct intp_s
{
    interval_t *i;
    uint32_t overlap;
    TAILQ_ENTRY(intp_s) ints;
    TAILQ_ENTRY(intp_s) act;
};

static TAILQ_HEAD(intlist, intp_s) intps;
                                   
typedef TAILQ_HEAD(activelist, intp_s) active_list_t;

#define NEW_INTP(t) do {                                \
if(NULL == (t = (intp_t*)calloc(1, sizeof(intp_t)))) {  \
    fprintf(stderr, "Memory allocation error.\n");      \
    exit(EXIT_FAILURE);                                 \
}                                                       \
} while(0)                                              \

int
intervals(bb_code_t *code)
{
    uint32_t c = 0;
    interval_t * i = NULL;
    
    list_each(code->blocks,blk)
    {
        list_each(blk->ins, ins)
        {
            ++c;
            if(tac_is_binop(ins->op))
            {
                if(ins->a1->type != SCONST)
                {
                    i = (interval_t *)ins->a1->aux;
                    ASSERT(i->start);
                    i->end = c;
                    ++i->usecount;
                }

                if(ins->a2->type != SCONST)
                {
                    i = (interval_t *)ins->a2->aux;
                    ASSERT(i->start);
                    i->end = c;
                    ++i->usecount;
                }

                i = (interval_t *)ins->r->aux;
                if (!i->start)
                {
                    i->start = c;
                    i->end = c;
                }
                else
                {
                    i->end = c;
                }
                ++i->usecount;
                continue;
            }
        
            if(ins->op == TIN)
            {
                i = (interval_t *)ins->r->aux;
                if (!i->start)
                {
                    i->start = c;
                    i->end = c;
                }
                else
                {
                    i->end = c;
                }
                ++i->usecount;
                continue;
            }

            if(ins->op == TOUT)
            {
                if(ins->r->type != SCONST)
                {
                    i = (interval_t *)ins->r->aux;
                    if(!i->start) TRACE(10, "!i-start symbol %s", ins->r->name);
                    ASSERT(i->start);
                    i->end = c;
                    ++i->usecount;
                }
                continue;
            }

            if(ins->op == TIFF)
            {
                if(ins->a1->type != SCONST)
                {
                    i = (interval_t *)ins->a1->aux;
                    ASSERT(i->start);
                    i->end = c;
                    ++i->usecount;
                }
                continue;
            }

            if(ins->op == TASSIGN)
            {
                if(ins->a1->type != SCONST)
                {
                    i = (interval_t *)ins->a1->aux;
                    ASSERT(i->start);
                    i->end = c;
                    ++i->usecount;
                }

                i = (interval_t *)ins->r->aux;
                if (!i->start)
                {
                    i->start = c;
                    i->end = c;
                }
                else
                {
                    i->end = c;
                }
                ++i->usecount;
                continue;
            }
        }
    }
    
    return 0;
}

void
alloc_intervals(void *symb)
{
    interval_t *i = NULL;
    
    if(((symbol_t*)symb)->type !=  SVAR) return;
    
    if(NULL == (((symbol_t*)symb)->aux = calloc(1, sizeof(interval_t))))
    {
        fprintf(stderr, "Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    i = (interval_t *)((symbol_t*)symb)->aux;
    i->s = (symbol_t *)symb;
}

void
print_intervals(void *symb)
{
    symbol_t *s = (symbol_t *)symb;
    interval_t *i = NULL;

    ASSERT(symb);
    
    if(s->type !=  SVAR) return;

    i = (interval_t *)s->aux;

    fprintf(out, "interval for %s\n",i->s->name);
    fprintf(out, "start %d - end %d : used %d -  register %d\n\n",i->start, i->end, i->usecount, i->reg);
}


void
unused_vars(void *symb)
{
    symbol_t *s = (symbol_t *)symb;
    interval_t *i = NULL;

    ASSERT(symb);
    
    if(s->type !=  SVAR) return;

    i = (interval_t *)s->aux;
    if (0 == i->usecount)
    {
        TRACE(15, "Unused variable %s - rem from symbol table", i->s->name);
        free(s->aux);
        st_rem_symbol(s);
    }
}


void
fill_interval_list(void *symb)
{
    symbol_t *s = (symbol_t *)symb;
    interval_t *i = NULL;
    intp_t *t = NULL;
    intp_t *n = NULL;

    ASSERT(symb);
    
    if(s->type !=  SVAR) return;

    i = (interval_t *)s->aux;

    NEW_INTP(t);
    t->i = i;

    if(TAILQ_EMPTY(&intps))
    {
        TAILQ_INSERT_HEAD(&intps, t, ints);
        return;
    }
    
    TAILQ_FOREACH(n, &intps, ints)
    {
        if(t->i->start < n->i->start)
        {
            TAILQ_INSERT_BEFORE(n, t, ints);
            return;
        }
    }

    TAILQ_INSERT_TAIL(&intps, t, ints);
}


stackt_t *
init_regs()
{
    stackt_t *regs = NULL;

    if(NULL == (regs = stack_init(regs, REGISTER_COUNT)))
    {
        fprintf(stderr,"Memory allocation failed.!");
        exit(EXIT_FAILURE);
    }

    for(int i = REGISTER_COUNT - 1; i >= 0; --i)
    {
        stack_push(regs, (void *)(intptr_t)i);
    }

    return regs;
}

int
lin_expire_old(intp_t *n, active_list_t *active, stackt_t *regs)
{
    intp_t *t = NULL;
    intp_t *s = NULL;
    
    TAILQ_FOREACH_SAFE(t, active, act, s)
    {
        if (t->i->end >= n->i->start) return 0;
        
        stack_push(regs, (void *)(intptr_t)t->i->reg);
        TAILQ_REMOVE(active, t, act);
    }

    return 0;
}

int
lin_add_active(intp_t *n, active_list_t *active)
{
    intp_t *t = NULL;
    
    if(TAILQ_EMPTY(active))
    {
        TAILQ_INSERT_HEAD(active, n, act);
        return 0;
    }
    
    TAILQ_FOREACH(t, active, act)
    {
        if(n->i->end < t->i->end)
        {
            TAILQ_INSERT_BEFORE(t, n, act);
            return 0;
        }
    }

    TAILQ_INSERT_TAIL(active, n, act);

    return 0;
}

int
lin_spill(intp_t *n, active_list_t *active)
{
    intp_t *last = TAILQ_LAST(active, activelist);

    if(last->i->end > n->i->end)
    {
        n->i->reg = last->i->reg;
        last->i->reg = NO_REGISTER;
        TAILQ_REMOVE(active, last, act);
        lin_add_active(n, active);
    }
    else
    {
        n->i->reg = NO_REGISTER;
    }

    return 0;
}

int
lin_scan(bb_code_t *code)
{
    intp_t *n = NULL;
    active_list_t active;
    stackt_t *regs = NULL;
    uint32_t memloc = 0;

    TRACE(15, "LINSCAN%s", "");

    TAILQ_INIT(&active);

    regs = init_regs();

    TAILQ_FOREACH(n, &intps, ints)
    {
        TRACE(15, "interval %s : %d -> %d", n->i->s->name, n->i->start, n->i->end);

        lin_expire_old(n, &active, regs);

        if(!stack_not_empty(regs))
        {
            TRACE(15, "Stack empty spill %s", n->i->s->name);
            lin_spill(n, &active);
        }
        else
        {
            n->i->reg  = (int) (intptr_t)stack_pop(regs);
            TRACE(15, "%s : reg %d", n->i->s->name, n->i->reg);
            lin_add_active(n, &active);
        }
    }

    TAILQ_FOREACH(n, &intps, ints)
    {
        if(n->i->reg == NO_REGISTER)
        {
            n->i->s->val = memloc++;
            n->i->s->reg = NO_REGISTER;
        }
        else
        {
            n->i->s->state = VS_ASSIGNED;
            n->i->s->val = NO_REGISTER;
            n->i->s->reg = n->i->reg;
        }
    }

    return 0;
}


bb_code_t *
reg_alloc(bb_code_t *code)
{
    TRACE(5, "Allocate registers ...%s", "");

    st_iterate(alloc_intervals);

    if(0 != intervals(code))
    {
        TRACE(5, "Intervals failed ...%s", "");
        return NULL;
    }

    st_iterate(unused_vars);

    TAILQ_INIT(&intps);
    
    st_iterate(fill_interval_list);

    if(0 != lin_scan(code))
    {
        TRACE(5, "Register allocation failed ...%s", "");
        return NULL;
    }

    /* st_iterate(print_intervals); */

    return code;
}
