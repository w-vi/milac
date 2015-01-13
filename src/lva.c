/***************************************************
* Live Variable Analysis for Mila compiler         *
***************************************************/
#include "global.h"
#include "util.h"
#include "3ac.h"
#include "bitset.h"


#define USEBIT(arg, b) do {                             \
if(arg->type != SCONST)                                 \
{                                                       \
    if(!BITTEST(b->lva.def, arg->id))                   \
    {                                                   \
        BITSET(b->lva.use, arg->id);                    \
    }                                                   \
}                                                       \
} while(0)                                              \

#define INUSE(arg, b) do {                              \
if(arg->type != SCONST)                                 \
{                                                       \
    BITSET((char *)b, arg->id);                         \
}                                                       \
} while(0)                                              \


char *
lva_init(bb_code_t *code)
{
    uint32_t symb_count = st_get_var_count();
    char * btmp = NULL;

    TRACE(15, "symbols count...%d", symb_count);

    list_each(code->blocks, blk)
    {
        blk->lva.count = symb_count;
        blk->lva.def = calloc(1, BITNSLOTS(symb_count));
        blk->lva.use = calloc(1, BITNSLOTS(symb_count));
        blk->lva.in = calloc(1, BITNSLOTS(symb_count));
        blk->lva.out = calloc(1, BITNSLOTS(symb_count));
        btmp = calloc(1, BITNSLOTS(symb_count));
        
        if(NULL ==  blk->lva.def || NULL ==  blk->lva.use ||
           NULL ==  blk->lva.in || NULL ==  blk->lva.out ||
           NULL == btmp)
        {
            return NULL;
        }
    }
    
    return btmp;
}


char *
lva_bb_init(bb_t *bb)
{
    uint32_t symb_count = st_get_var_count();
    char * btmp = NULL;

    TRACE(15, "symbols count...%d", symb_count);

    list_each(bb->ins, ins)
    {
        ins->in = calloc(1, BITNSLOTS(symb_count));
        ins->out = calloc(1, BITNSLOTS(symb_count));
        btmp = calloc(1, BITNSLOTS(symb_count));
        
        if(NULL ==  ins->in || NULL ==  ins->out ||
           NULL == btmp)
        {
            return NULL;
        }
    }
    
    return btmp;
}

void
lva_bb_delete(bb_t *bb)
{
    list_each(bb->ins, ins)
    {
        if(NULL != ins->in)
        {
            free(ins->in);
            ins->in = NULL;
        }
        if(NULL != ins->out)
        {
            free(ins->out);
            ins->out = NULL;
        }
    }
}


void
lva_delete(bb_code_t *code)
{
    list_each(code->blocks, blk)
    {
        if(NULL != blk->lva.def)
        {
            free(blk->lva.def);
            blk->lva.def = NULL;
        }
        
        if(NULL != blk->lva.use)
        {
            free(blk->lva.use);
            blk->lva.use = NULL;
        }
        if(NULL != blk->lva.in)
        {
            free(blk->lva.in);
            blk->lva.in = NULL;
        }
        if(NULL != blk->lva.out)
        {
            free(blk->lva.out);
            blk->lva.out = NULL;
        }
    }
}

void
defuse_bb(bb_t *b)
{
    list_each(b->ins, ins)
    {
        if(tac_is_binop(ins->op))
        {
            if(ins->a1->type != SCONST)
            {
                USEBIT(ins->a1, b);
            }

            if(ins->a2->type != SCONST)
            {
                USEBIT(ins->a2, b);
            }

            BITSET(b->lva.def, ins->r->id);
            continue;
        }
        
        if(ins->op == TIN) 
        {
            BITSET(b->lva.def, ins->r->id);
            continue;
        }

        if(ins->op == TOUT)
        {
            if(ins->r->type != SCONST)
            {
                USEBIT(ins->r, b);
            }
            continue;
        }

        if(ins->op == TIFF)
        {
            if(ins->a1->type != SCONST)
            {
                USEBIT(ins->a1, b);
            }
            continue;
        }

        if(ins->op == TASSIGN)
        {
            if(ins->a1->type != SCONST)
            {
                USEBIT(ins->a1, b);
            }
            
            BITSET(b->lva.def, ins->r->id);
            continue;
        }
    }
}

bb_code_t *
lva_defuse(bb_code_t *code)
{
    list_each(code->blocks, b)
    {
        defuse_bb(b);
    }

    return code;
}

void
lva_print(bb_t *bb)
{
    TRACE(15, "BB %d----------------------------", bb->bbid);

    for(int i = 0; i < bb->lva.count; ++i)
    {
        if(BITTEST((char *)bb->lva.out, i))
        {
            TRACE(15, "Symbol %d SET", i);
        }
        else
        {
            TRACE(15, "Symbol %d NOT", i);
        }
    }
}

bb_code_t *
lva(bb_code_t *code)
{
    int changed = TRUE;
    uint32_t i  = 0;
    bb_code_t *c = code;
    bb_t *succ = NULL;
    char *bcmp = NULL;

    TRACE(5, "Live Variable Analysis pass %s", "");

    ASSERT(NULL != code);

    lva_delete(c);

    bcmp = lva_init(c);
    if(NULL == bcmp)
    {
        lva_delete(c);
        return NULL;
    }

    
    c = lva_defuse(c);

    while(changed)
    {
        changed = FALSE;
        list_each_r(c->blocks, b)
        {
            TRACE(15, "lva run %d", b->bbid);

            memcpy(bcmp, b->lva.out, BITNSLOTS(b->lva.count));

            for(i = 0; i < b->lva.count; ++i)
            {
                if(BITTEST(b->lva.out, i) && !BITTEST(b->lva.def, i))
                {
                    BITSET(b->lva.in, i);
                }
                else
                {
                    if(BITTEST(b->lva.use, i))
                    {
                        BITSET(b->lva.in, i);
                    }
                    else
                    {
                        BITCLEAR(b->lva.in, i);
                    }
                }
            }
            
            succ = b->jmp;
            if(NULL !=  succ)
            {
                for(i = 0; i <  b->lva.count; ++i)
                {
                    if(BITTEST(succ->lva.in, i))
                    {
                        BITSET(b->lva.out, i);
                    }
                }
                
            }
            succ = b->next;
            if(NULL !=  succ)
            {
                for(i = 0; i <  b->lva.count; ++i)
                {
                    if(BITTEST(succ->lva.in, i))
                    {
                        BITSET(b->lva.out, i);
                    }
                }
            }

            for(i = 0; i < b->lva.count; ++i)
            {
                if(BITTEST(b->lva.out, i) && !BITTEST(bcmp, i))
                {
                    TRACE(15, "%d lva.changed ", b->bbid);
                    changed = TRUE;
                    break;
                }
                if(!BITTEST(b->lva.out, i) && BITTEST(bcmp, i))
                {
                    TRACE(15, "%d lva.changed ", b->bbid);
                    changed = TRUE;
                    break;
                }
            }
        }
    }

    list_each(c->blocks, b)
    {
        lva_print(b);
    }
    
    

    free(bcmp);

    return c;
}

void
lva_print_ins(tac_t *ins)
{
    print_tac(ins, 9999);
    
    for(int i = 0; i < ins->bb->lva.count; ++i)
    {
        if(BITTEST((char *)ins->out, i))
        {
            TRACE(15, "Symbol id %d SET", i);
        }
        else
        {
            TRACE(15, "Symbol id %d not", i);
        }
    }
}

int
lva_bb(bb_t *bb)
{
    char *bcmp = NULL;
    tac_t *prev = NULL;
    

    TRACE(15, "Live Variable Analysis pass %d", bb->bbid);

    ASSERT(NULL != bb);

    bcmp = lva_bb_init(bb);
    if(NULL == bcmp)
    {
        lva_bb_delete(bb);
        return -1;
    }

    prev = list_back(bb->ins);
    memcpy(prev->out, bb->lva.out, BITNSLOTS(bb->lva.count));

    list_each_r(bb->ins, ins)
    {
        if(prev == ins) continue;
        
        if(tac_is_binop(ins->op))
        {
            if(ins->a1->type != SCONST)
            {
                INUSE(ins->a1, prev->out);
            }

            if(ins->a2->type != SCONST)
            {
                INUSE(ins->a2, prev->out);
            }

            BITSET((char *)prev->out, ins->r->id);
        }
        
        if(ins->op == TIN)
        {
            BITSET((char *)prev->out, ins->r->id);
        }
        
        if(ins->op == TOUT)
        {
            if(ins->r->type != SCONST)
            {
                INUSE(ins->r, prev->out);
            }
        }

        if(ins->op == TIFF)
        {
            if(ins->a1->type != SCONST)
            {
                INUSE(ins->a1, prev->out);
            }
        }

        if(ins->op == TASSIGN)
        {
            if(ins->a1->type != SCONST)
            {
                INUSE(ins->a1, prev->out);
            }
            
            BITSET((char *)prev->out, ins->r->id);
        }

        memcpy(ins->out, prev->out, BITNSLOTS(bb->lva.count));
        prev  = ins;
    }

    return 0;
}
