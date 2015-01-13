/***************************************************
* Optimizations functions for Mila compiler        *
***************************************************/
#include "global.h"
#include "util.h"
#include "optimize.h"
#include "bitset.h"

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))

typedef bb_t* (*bbpass_fn)(bb_t *);

typedef bb_code_t* (*pass_fn)(bb_code_t *);

typedef union
{
    bbpass_fn bbfn;
    pass_fn fn;
} opt_function_t;

typedef struct opt_item_s
{
    opt_function_t func;
    int global;
} opt_item_t;

bb_t *
opt_algebra(bb_t *code)
{
    TRACE(5, "Algebraic optimization pass on bb %d", code->bbid);

    ASSERT(NULL != code);
    
    list_each(code->ins, ins)
    {
        if(tac_is_binop(ins->op))
        {
            switch(ins->op)
            {
            case TADD:
                if(ins->a1->type == SCONST || ins->a2->type == SCONST)
                {
                    if(ins->a1->type == SCONST && ins->a1->val == 0)
                    {
                        ins->op = TASSIGN;
                        ins->a1 = ins->a2;
                        ins->a2 = NULL;
                    }
                    else if(ins->a2->type == SCONST && ins->a2->val == 0)
                    {
                        ins->op = TASSIGN;
                        ins->a2 = NULL;
                    }
                }
                break;
            case TSUB:
                if(ins->a1->type == SCONST || ins->a2->type == SCONST)
                {
                    if(ins->a1->type == SCONST && ins->a1->val == 0)
                    {
                        ins->op = TMUL;
                        ins->a1 = st_insert_const(-1);
                    }
                    else if(ins->a2->type == SCONST && ins->a2->val == 0)
                    {
                        ins->op = TASSIGN;
                        ins->a2 = NULL;
                    }
                }
                break;
            case TMUL:
                if(ins->a1->type == SCONST || ins->a2->type == SCONST)
                {
                    if(ins->a1->type == SCONST && ins->a1->val == 0)
                    {
                        ins->op = TASSIGN;
                        ins->a1 = st_insert_const(0);
                        ins->a2 = NULL;
                    }
                    else if(ins->a2->type == SCONST && ins->a2->val == 0)
                    {
                        ins->op = TASSIGN;
                        ins->a1 = st_insert_const(0);
                        ins->a2 = NULL;
                    }
                    else if(ins->a1->type == SCONST && ins->a1->val == 1)
                    {
                        ins->op = TASSIGN;
                        ins->a1 = ins->a2;
                        ins->a2 = NULL;
                    }
                    else if(ins->a1->type == SCONST && ins->a1->val == 2)
                    {
                        ins->op = TADD;
                        ins->a1 = ins->a2;
                    }
                    else if(ins->a2->type == SCONST && ins->a2->val == 1)
                    {
                        ins->op = TASSIGN;
                        ins->a2 = NULL;
                    }
                    else if(ins->a2->type == SCONST && ins->a2->val == 2)
                    {
                        ins->op = TADD;
                        ins->a2 = ins->a1;
                    }
                }
                break;
            case TDIV:
                if(ins->a1->type == SCONST || ins->a2->type == SCONST)
                {
                    if(ins->a2->type == SCONST && ins->a2->val == 0)
                    {
                        fprintf(stderr, "Error : Possible division by zero!");
                    }
                    else if(ins->a1->type == SCONST && ins->a1->val == 1)
                    {
                        ins->op = TASSIGN;
                        ins->a1 = ins->a2;
                        ins->a2 = NULL;
                    }
                    else if(ins->a2->type == SCONST && ins->a2->val == 1)
                    {
                        ins->op = TASSIGN;
                        ins->a2 = NULL;
                    }
                }
                break;
            default:
                continue;
            }
        }
    }

    return code;
}

bb_code_t *
opt_bb_pass(bb_code_t *code, bbpass_fn fn)
{
    list_each(code->blocks, blk)
    {
        fn(blk);
    }
    return code;
}

bb_code_t *
opt_unused_vars(bb_code_t *code)
{
    uint32_t symb_count = st_get_var_count();
    char *use = NULL;
    tac_t *tac = NULL;
    
    TRACE(5, "Unused Variables pass %s", "");
    
    ASSERT(NULL != code);

    if(NULL == (use = calloc(1, BITNSLOTS(symb_count))))
    {
        fprintf(stderr, "Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }

    list_each(code->blocks, bb)
    {
        list_each(bb->ins, ins)
        {
            if(tac_is_binop(ins->op))
            {
                if(ins->a1->type != SCONST) BITSET(use, ins->a1->id);
                if(ins->a2->type != SCONST) BITSET(use, ins->a2->id);
                continue;
            }
        
            if(ins->op == TIN || ins->op == TOUT)
            {
                if(ins->r->type != SCONST) BITSET(use, ins->r->id);
                continue;
            }

            if(ins->op == TIFF)
            {
                if(ins->a1->type != SCONST) BITSET(use, ins->a1->id);
                continue;
            }

            if(ins->op == TASSIGN)
            {
                if(ins->a1->type != SCONST) BITSET(use, ins->a1->id);                
                continue;
            }
        }
    }

    list_each(code->blocks, bb)
    {
        list_each(bb->ins, ins)
        {
            if(ins->op == TASSIGN)
            {
                if(!BITTEST(use, ins->r->id)) ins->flags.set = 1;
            }
        }
    }

    list_each(code->blocks, bb)
    {
        list_each_elem(bb->ins, el)
        {
            tac = *el;

            if(tac->flags.set)
            {
                TRACE(15, "removing ...%s = %s %s unused", tac->r->name,
                      tac->a1->name, tac->r->name);
                list_elem_remove(el);
                tac_free(tac);
            }
        }
    }

    free(use);

    return code;
}

opt_item_t  opt_level_0[] = {{{.fn = lva}, TRUE}};

opt_item_t  opt_level_1[] = {{{.fn = const_prop}, TRUE},
                             {{.bbfn = opt_algebra}, FALSE},
                             {{.fn = opt_unused_vars}, TRUE}, 
                             {{.fn = lva}, TRUE},
                             {{.fn = copy_prop}, TRUE},
                             {{.fn = opt_unused_vars}, TRUE},
                             {{.fn = lva}, TRUE}};

opt_item_t  opt_level_2[] = {{{.fn = const_prop}, TRUE},
                             {{.bbfn = opt_algebra}, FALSE},
                             {{.fn = opt_unused_vars}, TRUE}, 
                             {{.fn = lva}, TRUE},
                             {{.fn = copy_prop}, TRUE},
                             {{.fn = opt_unused_vars}, TRUE},
                             {{.fn = lva}, TRUE},
                             {{.bbfn = cse}, FALSE},
                             {{.fn = opt_unused_vars}, TRUE},
                             {{.fn = lva}, TRUE}};

bb_code_t * optimize(bb_code_t *code, int32_t level)
{
    TRACE(5, "Machine independent optimization ... level %d", level);

    if(level == 0)
    {
        for(uint32_t i = 0; i < NELEMS(opt_level_0); ++i)
        {
            if(opt_level_0[i].global)
            {
                code = opt_level_0[i].func.fn(code);
            }
            else
            {
                code = opt_bb_pass(code, opt_level_0[i].func.bbfn);
            }
        }
    }

    if(level == 1)
    {
        for(uint32_t i = 0; i < NELEMS(opt_level_1); ++i)
        {
            if(opt_level_1[i].global)
            {
                code = opt_level_1[i].func.fn(code);
            }
            else
            {
                code = opt_bb_pass(code, opt_level_1[i].func.bbfn);
            }
        }
    }
    
    if(level > 1)
    {
        for(uint32_t i = 0; i < NELEMS(opt_level_2); ++i)
        {
            if(opt_level_2[i].global)
            {
                /* if(opt_level_1[i].func.fn == copy_prop) */
                /*     print_bb(code); */
                
                code = opt_level_2[i].func.fn(code);
                
                /* if(opt_level_1[i].func.fn == copy_prop) */
                /*     print_bb(code); */
            }
            else
            {
                /* if(opt_level_1[i].func.bbfn == cse) */
                /*     print_bb(code); */
                
                code = opt_bb_pass(code, opt_level_2[i].func.bbfn);
                
                /* if(opt_level_1[i].func.bbfn == cse) */
                /*     print_bb(code); */
            }
        }
    }
    
    return code;
}
