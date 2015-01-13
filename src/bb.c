/***************************************************
* Basic block functions for Mila compiler          *
***************************************************/
#include "global.h"
#include "util.h"
#include "3ac.h"

void
bb_free(bb_t * b)
{
    list_clear(b->ins);
    free(b);
}

void
compact_preds(bb_t *bb)
{
    uint32_t i = 0;
    uint32_t last = 0;

    for(i = 0 ; i <  MAX_PREDECESSORS; ++i)
    {
        if(NULL != bb->pred[i])
        {
            if(last != i - 1 && 0 != i)
            {
                bb->pred[last + 1] = bb->pred[i];
                ++last;
            }
            else
                last = i;
        }
    }
}

void
bb_remove_predecessors(bb_code_t *code, bb_t *rem)
{
    uint32_t i = 0;
    
    list_each(code->blocks, b)
    {
        if(b->flags.unreachable)
            continue;
        i = 0;
        while(b->pred[i] != NULL)
        {
            if(b->pred[i] == rem)
            {
                b->pred[i] = NULL;
                break;
            }
            ++i;
        }
        compact_preds(b);
    }
}

void
bb_add_predecessor(bb_t *dst, bb_t *src)
{
    uint32_t i = 0;

    if(NULL != dst)
    {
        for(i = 0 ; i <  MAX_PREDECESSORS; ++i)
        {
            if(NULL == dst->pred[i])
            {
                dst->pred[i] = src;
                break;
            }
        }
    }
    
}

uint32_t
bb_predecessor_count(bb_t *b)
{
    uint32_t count = 0;

    ASSERT(NULL != b);

    for(uint32_t i = 0; i < MAX_PREDECESSORS; ++i)
    {
        if(NULL != b->pred[i])
            count++;
    }

    return count;
}

bb_code_t *
bb_compact(bb_code_t *code, int *changed)
{
    uint32_t i = 0;
    bb_t *b, *p  = NULL;
    
    list_each_elem(code->blocks, el)
    {
        i = 0;
        b = *el;
        if(b->flags.unreachable)
        {
            TRACE(15, "doing block bb_%d", b->bbid);
            bb_remove_predecessors(code, b);
                
            *changed = TRUE;

            i = 0;
            p = b->pred[i];
            while(p != NULL)
            {
                if(p->next == b)
                {
                    p->next = b->next;
                }
                
                if(p->jmp == b)
                {
                    p->jmp = b->jmp;
                }
                
                ++i;
                p = b->pred[i];
            }
        }
    }

    list_each_elem(code->blocks, el)
    {
        b = *el;
        if(b->flags.unreachable)
        {
            list_elem_remove(el);
            bb_free(b);
        }
    }
    
    list_each_elem(code->blocks, el)
    {
        bb_t *b = *el;
        tac_t *ins = list_back(b->ins);
        bb_t *next = NULL;

        if(b->flags.unreachable)
            continue;

        if(ins->op == TIFF && NULL == b->jmp)
        {
            *changed = TRUE;
            TRACE(10, "jmp null - Remove if and merge blocks %d %d", b->bbid, b->next->bbid);
            free(ins->in);
            free(ins->out);
            free(ins);
            list_pop_back(b->ins);
            next =  b->next;
            
            list_each(next->ins, nins)
            {
                list_push(b->ins, nins);
            }
            
            b->next = next->next;
            b->jmp = next->jmp;
            if(b->next != NULL)
                TRACE(10, "New next is block %d", b->next->bbid);
            if(b->jmp != NULL)
                TRACE(10, "New jmp is block %d", b->jmp->bbid);
            next->flags.unreachable = 1;
            bb_remove_predecessors(code, next);
            bb_add_predecessor(b->next, b);
            continue;
        }

        if(ins->op == TIFF && NULL == b->next)
        {
            *changed = TRUE;
            TRACE(10, "next null - Remove if and merge blocks %d %d", b->bbid, b->jmp->bbid);
            free(ins->in);
            free(ins->out);
            free(ins);
            list_pop_back(b->ins);
            
            next =  b->jmp;
            ins = list_front(next->ins);
            free(ins->in);
            free(ins->out);
            free(ins);
            list_pop_front(next->ins);
            
            list_each(next->ins, nins)
            {
                list_push(b->ins, nins);
            }
           
            b->next = next->next;
            b->jmp = next->jmp;
            if(b->next != NULL)
                TRACE(10, "New next is block %d", b->next->bbid);
            if(b->jmp != NULL)
                TRACE(10, "New jmp is block %d", b->jmp->bbid);
            next->flags.unreachable = 1;
            bb_remove_predecessors(code, next);
            bb_add_predecessor(b->next, b);
            continue;
        }
        
        if(NULL != list_elem_next(el))
        {
            next = *list_elem_next(el);
        }
        
        if(ins->op == TJMP && (next == b->jmp) && (NULL != b->jmp))
        {
            *changed = TRUE;
            TRACE(10, "Remove jmp and merge blocks %d %d", b->bbid, b->jmp->bbid);
            free(ins->in);
            free(ins->out);
            free(ins);
            next =  b->jmp;
            list_pop_back(b->ins);
            list_pop_front(b->jmp->ins);
            
            list_each(next->ins, nins)
            {
                list_push(b->ins, nins);
            }
           
            b->next = next->next;
            b->jmp = next->jmp;
            if(b->next != NULL)
                TRACE(10, "New next is block %d", b->next->bbid);
            if(b->jmp != NULL)
                TRACE(10, "New jmp is block %d", b->jmp->bbid);
            next->flags.unreachable = 1;
            bb_remove_predecessors(code, next);
            bb_add_predecessor(b->next, b);
        }
    }

    list_each_elem(code->blocks, el)
    {
        b = *el;
        if(b->flags.unreachable)
        {
            list_elem_remove(el);
            bb_free(b);
        }
    }

    list_each_elem(code->blocks, el)
    {
        b = *el;
        if(1 == bb_predecessor_count(b))
        {
            tac_t *ins = list_front(b->ins);
            bb_t *pred = NULL;

            if((b == b->pred[0]->next) && (ins->op == TLABEL))
            {
                TRACE(10, "Unlabel merge blocks %d %d", b->bbid, b->pred[0]->bbid);
                *changed = TRUE;
                pred = b->pred[0];
                free(ins->in);
                free(ins->out);
                free(ins);
                list_pop_front(b->ins);
                list_each(b->ins, nins)
                {
                    list_push(pred->ins, nins);
                }
                
                pred->next = b->next;
                pred->jmp = b->jmp;
                if(pred->next != NULL)
                    TRACE(10, "New next is block %d", pred->next->bbid);
                if(pred->jmp != NULL)
                    TRACE(10, "New jmp is block %d", pred->jmp->bbid);
                b->flags.unreachable = 1;
                bb_remove_predecessors(code, b);
                bb_add_predecessor(pred->next, pred);                
            }
        }
    }

    list_each_elem(code->blocks, el)
    {
        b = *el;
        if(b->flags.unreachable)
        {
            list_elem_remove(el);
            bb_free(b);
        }
    }
    
    return code;
}

void
bb_print(bb_t *b)
{
    uint32_t i = 1;
    
    fprintf(out, "\nbb_%d\n\n", b->bbid);
    list_each(b->ins, in)
    {
        print_tac(in, i);
        ++i;
    }
    
    if(NULL != b->jmp)
        fprintf(out, "jmp -> bb_%d\n", b->jmp->bbid);
    if(NULL != b->next)
        fprintf(out, "next -> bb_%d\n", b->next->bbid);
    
    i = 0;
    fprintf(out, "predecessors \n");
    while(b->pred[i] != NULL)
    {
        fprintf(out, " bb_%d ", b->pred[i]->bbid);
        ++i;
    }
    fprintf(out, " \n");
}
