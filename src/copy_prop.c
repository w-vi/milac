/***************************************************
* Copy propagation framework for Mila compiler     *
***************************************************/
#include "global.h"
#include "util.h"
#include "optimize.h"
#include "bitset.h"

int
cpp_replace(bb_t *bb, tac_t *i)
{
    int replacing = FALSE;
    symbol_t *r = i->r;
    symbol_t *a = i->a1;

    list_each(bb->ins, ins)
    {
        if(replacing)
        {
            if(ins->op == TASSIGN)
            {
                if(ins->r == r && ins != i)
                {
                    TRACE(15, "new assignment ... stop replacing%s", "");
                    break;
                }
                continue;
            }

            if(tac_is_binop(ins->op))
            {
                if(ins->r == r)
                {
                    TRACE(15, "new + ... stop replacing%s", "");
                    break;
                }
                
                if(ins->a1 == r) ins->a1 = a;
                if(ins->a2 == r) ins->a2 = a;
                continue;
            }
        
            if(ins->op == TOUT)
            {
                if(ins->r == r) ins->r = a;
                continue;
            }
            
            if(ins->op == TIN)
            {
                if(ins->r == r)
                {
                    TRACE(15, "new read ... stop replacing%s", "");
                    break;
                }
                continue;
            }

            if(ins->op == TIFF)
            {
                if(ins->a1 == r) ins->a1 = a;
                continue;
            }
        }
        
        if(i == ins) replacing = TRUE;
    }

    return replacing;
}

int
cpp_bb(bb_t *bb)
{
    int changed = FALSE;
    tac_t * tac = NULL;
    
    ASSERT(NULL != bb);
    ASSERT(NULL != bb->lva.out);
    
    list_each(bb->ins, ins)
    {
        if(ins->op == TASSIGN && ins->a1->type != SCONST)
        {
            if(!BITTEST(bb->lva.out, ins->r->id))
            {
                if(tac != NULL && tac->r == ins->a1)
                {
                    TRACE(15, " has been replace already? %s = %s", tac->r->name, ins->r->name);
                    ins->bb = NULL;
                }
                
                TRACE(15, "possible copy prop? %s = %s", ins->r->name, ins->a1->name);
                cpp_replace(bb, ins);
            }
            else if(tac != NULL && tac->r == ins->a1
                    && BITTEST(bb->lva.out, ins->r->id))
            {
                TRACE(15, "possible? %s = %s", tac->r->name, ins->r->name);
                ins->bb = NULL;
                tac->r = ins->r;
                changed = TRUE;
            }
        }
       tac = ins;
        
    }

    list_each_elem(bb->ins, el)
    {
        tac = *el;

        if(tac->bb == NULL)
        {
            TRACE(15, "removing ...%s","");
            list_elem_remove(el);
        }
    }

    return changed;
}

bb_code_t *
copy_prop(bb_code_t *code)
{
    int changed = TRUE;
    
    TRACE(5, "Copy propagation pass%s", "");
    
    ASSERT(NULL != code);

    while(changed)
    {
        changed = FALSE;
        list_each(code->blocks, b)
        {
            changed = cpp_bb(b);
        }
    }

    return code;
}
