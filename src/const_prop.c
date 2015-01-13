/***************************************************
* Constant propagation framework for Mila compiler *
***************************************************/
#include "global.h"
#include "util.h"
#include "optimize.h"

typedef enum
{
    UNDEF = 0,
    NAC,
    C
} cp_var_state;

typedef struct cp_var_s
{
    cp_var_state c;
    symbol_t *s;
} cp_var_t;


static cp_var_t * tmp_vars = NULL;

void
cp_init_vars(void *symb)
{
    symbol_t *s = symb;
    
    if(s->type == SCONST)
    {
        tmp_vars[s->id].c = C;
        tmp_vars[s->id].s = s;
    }
}


static int
cp_init(bb_code_t *code)
{
    uint32_t symb_count = st_get_var_count();

    TRACE(15, "symbols count...%d", symb_count);

    list_each(code->blocks, blk)
    {
        TRACE(15, "blk %d", blk->bbid);

        list_each(blk->ins, ins)
        {
            ins->out = realloc(ins->out, symb_count * sizeof(cp_var_t));
            tmp_vars = realloc(tmp_vars, symb_count * sizeof(cp_var_t));
            
            if(NULL == ins->out || NULL == tmp_vars) return FALSE;
        }
    }
    
    return TRUE;
}


static int
cp_delete(bb_code_t *code)
{
    list_each(code->blocks, bb)
    {
        list_each(bb->ins, ins)
        {
            if(NULL != ins->out)
            {
                free(ins->out);
                ins->out = NULL;
            }
        }
    }

    free(tmp_vars);
    tmp_vars = NULL;

    return TRUE;
}

cp_var_t *
bb_out(bb_t *bb)
{
    tac_t * t = list_back(bb->ins);
    return t->out;
}


static int
cp_propagate(bb_code_t *code)
{
    uint32_t i = 0;
    bb_t *b = NULL;
    uint32_t symb_count = st_get_var_count();
    tac_t *ins2 = NULL;
    cp_var_t * in = NULL;
    cp_var_t * out = NULL;
    
    list_each(code->blocks, bb)
    {
        list_each(bb->ins, ins)
        {
            memset(ins->out, 0, (symb_count * sizeof(cp_var_t)));
        }
    }
    
    memset(tmp_vars, 0, (symb_count * sizeof(cp_var_t)));

    st_iterate(cp_init_vars);
    
    list_each(code->blocks, bb)
    {
        /*
         * Initialize the first in on the predecessors of bb
         */
        tac_t * ins = list_front(bb->ins);
        i = 0;
        b = bb->pred[i];
        while(NULL != b)
        {
            cp_var_t * p = bb_out(b);
            
            for(uint32_t k = 0; k < symb_count; ++k)
            {
                if(p[k].c == NAC)
                {
                    if(tmp_vars[k].c != NAC)
                    {
                        tmp_vars[k].c = NAC;
                    }
                }

                if(p[k].c == C)
                {
                    if(tmp_vars[k].c == C &&
                       (p[k].s != tmp_vars[k].s))
                    {
                        tmp_vars[k].c = NAC;
                    }

                    if(tmp_vars[k].c == UNDEF)
                    {
                        tmp_vars[k].c = C;
                        tmp_vars[k].s = p[k].s;
                    }
                }
            }
            ++i;
            b = bb->pred[i];
        }

        in = tmp_vars;
        memcpy(ins->out, tmp_vars, (symb_count * sizeof(cp_var_t)));
        
        /*
         * Process the BB
         */
        ins2 = NULL;
        list_each(bb->ins, ins)
        {
            out = (cp_var_t *)ins->out;
            
            if(NULL != ins2)
            {
                in = ins2->out;
                memcpy(ins->out, in, (symb_count * sizeof(cp_var_t)));
            }

            if(ins->op == TIN && out[ins->r->id].c != NAC)
            {
                out[ins->r->id].c = NAC;
            }

            if(ins->op == TASSIGN)
            {
                if(in[ins->a1->id].c == C)
                {
                    out[ins->r->id] = in[ins->a1->id];
                }
            }

            if(tac_is_binop(ins->op))
            {
                TRACE(15, "%s = %d  x %s = %d",
                      ins->a1->name,in[ins->a1->id].c,
                      ins->a2->name,in[ins->a2->id].c);

                if(in[ins->a1->id].c == C && NULL != in[ins->a1->id].s)
                {
                    ins->a1 = in[ins->a1->id].s;
                }

                if(in[ins->a2->id].c == C && NULL != in[ins->a2->id].s)
                {
                    ins->a2 = in[ins->a2->id].s;
                }

                if(in[ins->a1->id].c == C && in[ins->a2->id].c == C)
                {
                    out[ins->r->id].c = C;
                    out[ins->r->id].s = NULL;
                }
                    
                if(in[ins->a1->id].c == NAC ||  in[ins->a2->id].c == NAC)
                {
                    out[ins->r->id].c = NAC;
                }
            }
            ins2 = ins;
        }
    }

    return 0;
}

tac_t *
binop_fold(tac_t *ins, tac_t *prev)
{
    tac_t * new = NULL;
    int32_t res = 0;
    symbol_t * a1 = NULL;
    symbol_t * a2 = NULL;
    uint32_t symb_count = st_get_var_count();
    
    TRACE(15, "Folding %s", "");

    a1 = ((cp_var_t*)prev->out)[ins->a1->id].s;
    a2 = ((cp_var_t*)prev->out)[ins->a2->id].s;

    TRACE(15, "a1 = %d, a2 = %d", a1->val, a2->val);

    switch (ins->op)
    {
    case TADD:
        res = a1->val + a2->val;
        break;
    case TSUB:
        res = a1->val - a2->val;
        break;
    case TMUL:
        res = a1->val * a2->val;
        break;
    case TDIV:
        res = a1->val / a2->val;
        break;
    case TEQ:
        res = a1->val == a2->val;
        break;
    case TNEQ:
        res = a1->val != a2->val;
        break;
    case TLT:
        res = a1->val < a2->val;
        break;
    case TGT:
        res = a1->val > a2->val;
        break;
    case TLE:
        res = a1->val <= a2->val;
        break;
    case TGE:
        res = a1->val >= a2->val;
        break;
    default:
        ASSERT(0);
        fprintf(stderr, "Impossible %d\n", res);
        exit(0);
        break;
    }
    NEW_TAC(new);
    new->op = TASSIGN;
    new->a1 = st_insert_const(res);
    new->r = ins->r;
    new->in = NULL;
    new->out = calloc(symb_count, sizeof(cp_var_t));
    return new;
}


bb_code_t *
cp_fold(bb_code_t *code, int *changed)
{
    tac_t *ins_new = NULL;
    tac_t *prev = NULL;

    *changed = FALSE;

    TRACE(15, "Folding constants %s","");

    list_each(code->blocks, bb)
    {
        list_each_elem(bb->ins, el)
        {
            tac_t *ins = *el;

            if(ins->op == TIN ||
               ins->op == TJMP ||
               ins->op == TLABEL )
            {
                prev = ins;
                continue;
            }
            
            if(tac_is_binop(ins->op))
            {
                if((((cp_var_t*)ins->out)[ins->r->id].c == C) &&
                   (NULL == ((cp_var_t*)ins->out)[ins->r->id].s))
                {
                    TRACE(15, "changed%s", "");

                    *changed = TRUE;
                    ins_new = binop_fold(ins, prev);
                    *el = ins_new;
                    free(ins->in);
                    free(ins->out);
                    free(ins);
                    prev = ins_new;
                    break;
                }
            }
            
            if(ins->op == TASSIGN)
            {
                /* First ins might be assignment to SCONST */
                if(prev == NULL) 
                {
                    prev = ins;
                    continue;
                }
                
                if(((cp_var_t*)prev->out)[ins->a1->id].c == C &&
                   ins->a1 != ((cp_var_t*)prev->out)[ins->a1->id].s )
                {
                    TRACE(15, "changed%s", "");
                    *changed = TRUE;
                    ins->a1 = ((cp_var_t*)prev->out)[ins->a1->id].s;
                    break;
                }
                prev = ins;
                continue;
            }

            if(ins->op == TIFF)
            {
                if(((cp_var_t*)prev->out)[ins->a1->id].c == C)
                {
                    if(((cp_var_t*)prev->out)[ins->a1->id].s->val > 0)
                    {
                        bb->jmp->flags.unreachable = 1;
                    }
                    else
                    {
                        TRACE(15, "bb id %d unreachable", bb->next->bbid);
                        bb->next->flags.unreachable = 1;
                    }
                    TRACE(15, "changed%s", "");
                    *changed = TRUE;
                    prev = ins;
                    break;
                }
            }

            if(ins->op == TOUT)
            {
                if(((cp_var_t*)prev->out)[ins->r->id].c == C &&
                ins->r != ((cp_var_t*)prev->out)[ins->r->id].s)
                {
                    TRACE(15, "changed%s", "");
                    *changed = TRUE;
                    ins->r = ((cp_var_t*)prev->out)[ins->r->id].s;
                    prev = ins;
                    break;
                }
            }
        }
        if(*changed)
            break;
    }

    code = bb_compact(code, changed);

    return code;
}

bb_code_t * const_prop(bb_code_t *code)
{
    int changed = TRUE;
    bb_code_t *c = code;

    TRACE(5, "Constant propagation pass %s","");

    ASSERT(NULL != code);
    
    if(!cp_init(c))
    {
        cp_delete(c);
        return NULL;
    }

    while(changed)
    {
        changed = FALSE;
        cp_propagate(c);
        c = cp_fold(c, &changed);
        TRACE(15, "changed %d", changed);
        if(!cp_init(c))
        {
            cp_delete(c);
            return NULL;
        }
    }
    
    cp_delete(c);
    
    return c;
}
