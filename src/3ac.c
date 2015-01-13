/***************************************************
* 3ac code generation functions for Mila compiler  *
***************************************************/
#include "global.h"
#include "util.h"
#include "3ac.h"
#include "mila.tab.h"

static uint32_t bb_ids = 0;

list(tac_t*, tac_code);

#define NEW_BB(b) do {                                  \
if(NULL == (b = (bb_t*)calloc(1, sizeof(bb_t)))) {      \
    fprintf(stderr, "Memory allocation error.\n");      \
    exit(EXIT_FAILURE);                                 \
}                                                       \
b->bbid = bb_ids++;                                     \
} while(0)                                              \


#define ADD_PREDECESSEOR(b, p) do{              \
for(int i = 0; i < MAX_PREDECESSORS; ++i)       \
{                                               \
    if(NULL == b->pred[i])                      \
    {                                           \
      b->pred[i] = p;                           \
      break;                                    \
    }                                           \
}                                               \
} while(0)                                      \
    
#define PRINT_TAC_ARG(a) fprintf(out, " %s ", a->name) \


static void gen(tree_node_t * tree, tac_code_t *code);

void
print_tac(tac_t * ins, uint32_t i)
{
    ASSERT(NULL != ins);
    
    fprintf(out, "%d: ", i);

    switch(ins->op)
    {
    case TIFF:
        fprintf(out, "if false %s goto %s\n", ins->a1->name, ins->r->name);
        break;
    case TJMP:
        fprintf(out, "goto %s\n", ins->r->name);
        break;
    case TIN:
        fprintf(out, "read %s\n", ins->r->name);
        break;
    case TOUT:
        fprintf(out, "write ");
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "\n");
        break;
    case TEQ:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "==");
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");
        break;
    case TNEQ:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "!=");
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");
        break;
    case TLT:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "<");
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");
        break;
    case TGT:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, ">");
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");        
        break;
    case TLE:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, ">=");
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");
        break;
    case TGE:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "<=");
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");        
        break;
    case TASSIGN:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, ":=");
        PRINT_TAC_ARG(ins->a1);
        fflush(out);
        fprintf(out, "\n");    
        break;
    case TADD:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "+");    
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");    
        break;
    case TSUB:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "-");    
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");            
        break;
    case TMUL:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "*");    
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");    
        break;
    case TDIV:
        PRINT_TAC_ARG(ins->r);
        fprintf(out, "=");
        PRINT_TAC_ARG(ins->a1);
        fprintf(out, "/");    
        PRINT_TAC_ARG(ins->a2);
        fprintf(out, "\n");    
        break;
    case TLABEL:
        fprintf(out, "%s : \n", ins->r->name);
        break;
    default:
        fprintf(out, "UNKNOWN INSTRUCTION\n");
    }
}

void
print_tac_code(tac_code_t *code)
{
    uint32_t i = 1;

    list_each(code->ins, in)
    {
        print_tac(in, i);
        ++i;
    }
}


void
tac_gen(tree_node_t *t)
{
    if(t->type == BIOP_T)
    {
        switch(t->attr.op)
        {
        case PLUS:
            t->tac.op = TADD;
            break;
        case MINUS:
            t->tac.op = TSUB;
            break;
        case TIMES:
            t->tac.op = TMUL;
            break;
        case DIVIDE:
            t->tac.op = TDIV;
            break;
        case EQ:
            t->tac.op = TEQ;
            break;
        case NEQ:
            t->tac.op = TNEQ;
            break;
        case LT:
            t->tac.op = TLT;
            break;
        case GT:
            t->tac.op = TGT;
            break;
        case LTE:
            t->tac.op = TLE;
            break;
        case GTE:
            t->tac.op = TGE;
            break;
        default:
            break;
        }
        t->tac.a1 = t->child[0]->tac.r;
        t->tac.a2 = t->child[1]->tac.r;
        t->tac.r = st_temp_symbol(SVAR);
    }

    if(t->type == UOP_T)
    {
        t->tac.op = TMUL;
        t->tac.a1 = t->child[0]->tac.r;
        t->tac.a2 = st_insert_const(-1);
        t->tac.r = st_temp_symbol(SVAR);
    }

    if(t->type == IF_T)
    {
        t->tac.op = TIFF;
        t->tac.a1 = t->child[0]->tac.r;
        if(t->child[2] != NULL)
            t->tac.a2 = st_temp_symbol(SLABEL);
        else
            t->tac.a2 = NULL;
        t->tac.r = st_temp_symbol(SLABEL);
    }

    if(t->type == WHILE_T)
    {
        t->tac.op = TIFF;
        t->tac.a1 = t->child[0]->tac.r;
        t->tac.a2 = st_temp_symbol(SLABEL);
        t->tac.r = st_temp_symbol(SLABEL);
    }
}

static void
gen_statm(tree_node_t *t, tac_code_t *code)
{
    tac_t *i = NULL;

    ASSERT(NULL != t);
    ASSERT(NULL != code);

    NEW_TAC(i);

    if(t->type != READ_T)
    {
        if(!t->child[0]->flags.leaf)
            gen(t->child[0], code);
    }

    *i = t->tac;
    i->id = ++code->count;
    list_push(code->ins, i);
}

static void
gen_unop(tree_node_t *t, tac_code_t *code)
{
    tac_t *i = NULL;

    ASSERT(NULL != t);
    ASSERT(NULL != code);

    NEW_TAC(i);
    
    if(!t->child[0]->flags.leaf)
        gen(t->child[0], code);

    *i = t->tac;
    i->id = ++code->count;
    list_push(code->ins, i);
}

static void
gen_binop(tree_node_t *t, tac_code_t *code)
{
    tac_t *i = NULL;

    ASSERT(NULL != t);
    ASSERT(NULL != code);
    
    NEW_TAC(i);
    
    if(!t->child[0]->flags.leaf)
        gen(t->child[0], code);
    
    if(!t->child[1]->flags.leaf)
        gen(t->child[1], code);

    *i = t->tac;
    i->id = ++code->count;
    list_push(code->ins, i);
}

static void
gen_if(tree_node_t *t, tac_code_t *code)
{
    tac_t *i = NULL;

    ASSERT(NULL != t);
    ASSERT(NULL != code);
    
    NEW_TAC(i);
    
    if(!t->child[0]->flags.leaf)
        gen(t->child[0], code);

    *i = t->tac;
    i->id = ++code->count;
    list_push(code->ins, i);
    
    gen(t->child[1], code);
    
    if(t->child[2] != NULL)
    {
        NEW_TAC(i);
        
        i->op = TJMP;
        i->r = t->tac.a2;
        i->id = ++code->count;
        list_push(code->ins, i);

        NEW_TAC(i);

        i->op = TLABEL;
        i->r = t->tac.r;
        i->id = ++code->count;
        list_push(code->ins, i);
        
        gen(t->child[2], code);

        NEW_TAC(i);

        i->op = TLABEL;
        i->r = t->tac.a2;
        i->id = ++code->count;
        list_push(code->ins, i);
    }
    else
    {
        NEW_TAC(i);

        i->op = TLABEL;
        i->r = t->tac.r;
        i->id = ++code->count;
        list_push(code->ins, i);
    }
}

static void
gen_loop(tree_node_t *t, tac_code_t *code)
{
    tac_t *i = NULL;
    
    ASSERT(NULL != t);
    ASSERT(NULL != code);
    
    NEW_TAC(i);
    
    i->op = TLABEL;
    i->r = t->tac.a2;
    i->id = ++code->count;
    list_push(code->ins, i);

    if(!t->child[0]->flags.leaf)
        gen(t->child[0], code);

    NEW_TAC(i);

    *i = t->tac;
    i->id = ++code->count;
    list_push(code->ins, i);
        
    gen(t->child[1], code);

    NEW_TAC(i);
    
    i->op = TJMP;
    i->r = t->tac.a2;
    i->id = ++code->count;
    list_push(code->ins, i);

    NEW_TAC(i);
    
    i->op = TLABEL;
    i->r = t->tac.r;
    i->id = ++code->count;
    list_push(code->ins, i);
}

static void
gen(tree_node_t * tree, tac_code_t *code)
{
    if(tree != NULL)
    {
        if(tree->klass == CL_STATEMENT)
        {
            switch(tree->type)
            {
            case IF_T:
                gen_if(tree, code);
                break;
            case WHILE_T:
                gen_loop(tree, code);
                break;
            case READ_T:
            case WRITE_T:
            case ASSIGN_T:
                gen_statm(tree, code);
            default:
                break;
            }
        }
        else
        {
            switch(tree->type)
            {
            case BIOP_T:
                gen_binop(tree, code);
                break;
            case UOP_T:
                gen_unop(tree, code);
                break;
            default:
                break;
            }
            
        }
        gen(tree->sibling, code);
    }
}

tac_code_t *
tac(tree_node_t *t)
{
    tac_code_t *code = (tac_code_t *)malloc(sizeof(tac_code_t));
    if(NULL == code)
    {
        fprintf(stderr, "Memory error\n");
        return NULL;
    }
    memset(&tac_code, 0, sizeof (tac_code));    
    memset(&code->ins, 0, sizeof (code->ins));    
    code->count = 0;

    TRACE(5, "Starting 3AC code generation ...%s", "");
    gen(t, code);
    TRACE(5, "Created %d instructions ...", code->count);

    /* finish */
    TRACE(5, "3AC code generation finished. %s", "");

    return code;
}

bb_code_t *
bb_build(tac_code_t *code)
{
    bb_t *bb = NULL;
    bb_t *bb1 = NULL;

    bb_code_t *c = (bb_code_t *)calloc(1, sizeof(bb_code_t));
    if(NULL == c)
    {
        fprintf(stderr, "Memory error\n");
        return NULL;
    }
    memset(&c->blocks, 0, sizeof (c->blocks));    
    TRACE(5, "Building basic blocks  ...%s", "");

    NEW_BB(bb);
    list_push(c->blocks, bb);
    bb->id = UINT32_MAX;
    ++c->count;

    list_each(code->ins, in)
    {
        if(in->op == TLABEL || in->op == TIFF)
        {
            NEW_BB(bb);
            if(in->op == TLABEL)
                bb->id = in->r->val;

            list_push(c->blocks, bb);
            ++c->count;
        }
    }

    TRACE(5, "Created %d basic blocks.", c->count);
        
    bb1 = *list_elem_front(c->blocks);
    list_each(code->ins, tmp)
    {
        if(bb1->ins == NULL)
        {
            tmp->bb = bb1;
            list_push(bb1->ins, tmp);
            continue;
        }

        if(tmp->op == TLABEL)
        {
            list_each(c->blocks, blk)
            {
                if(blk->id == tmp->r->val)
                {
                    bb = blk;
                    break;
                }
            }
            tmp->bb = bb;
            list_push(bb->ins, tmp);
            bb1->next = bb;
            ADD_PREDECESSEOR(bb, bb1);
            bb1 = bb;
            continue;
        }

        if(tmp->op == TJMP)
        {
            tmp->bb = bb1;
            list_push(bb1->ins, tmp);

            list_each(c->blocks, blk)
            {
                if(blk->id == tmp->r->val)
                {
                    bb1->jmp = blk;
                    bb1->next = NULL;
                    ADD_PREDECESSEOR(blk, bb1);
                    break;
                }
            }

            if(bb1->jmp == NULL)
            {
                fprintf(stderr, "IMPOSSIBLE");
                exit(EXIT_FAILURE);
            }

            list_each_elem(c->blocks, e)
            {
                if(*e == bb1)
                {
                    bb1 = *list_elem_next(e);
                    break;
                }
            }
            continue;
        }

        if(tmp->op == TIFF)
        {
            tmp->bb = bb1;
            list_push(bb1->ins, tmp);

            list_each(c->blocks, blk)
            {
                if(blk->id == tmp->r->val)
                {
                    bb1->jmp = blk;
                    ADD_PREDECESSEOR(blk, bb1);
                    break;
                }
            }

            list_each_elem(c->blocks, e)
            {
                if(*e == bb1)
                {
                    bb = *list_elem_next(e);
                    break;
                }
            }
            
            bb1->next = bb;
            ADD_PREDECESSEOR(bb, bb1);
            bb1 = bb;
            continue;
        }
        tmp->bb = bb1;
        list_push(bb1->ins, tmp);
    }

    TRACE(5, "Basic blocks built. %s", "");

    list_clear(code->ins);
    free(code);
    return c;
}

int
tac_is_binop(codeop_type op)
{
    switch(op)
    {
    case TEQ:
    case TNEQ:
    case TLT:
    case TGT:
    case TLE:
    case TGE:
    case TADD:
    case TSUB:
    case TMUL:
    case TDIV:
        return TRUE;
    default:
        return FALSE;
    }
}

void
print_bb(bb_code_t *code)
{
    uint32_t i = 1;

    list_each(code->blocks, tmp)
    {
        fprintf(out, "\nbb_%d\n\n", tmp->bbid);

        list_each(tmp->ins, in)
        {
            print_tac(in, i);
            ++i;
        }
         
        if(NULL != tmp->jmp)
            fprintf(out, "jmp -> bb_%d\n", tmp->jmp->bbid);
        if(NULL != tmp->next)
            fprintf(out, "next -> bb_%d\n", tmp->next->bbid);
        
        i = 0;
        fprintf(out, "predecessors \n");
        while(tmp->pred[i] != NULL)
        {
            fprintf(out, " bb_%d ", tmp->pred[i]->bbid);
            ++i;
        }
        fprintf(out, " \n");
    }
}


void
print_bb_to_dot(bb_code_t *code, FILE *dot)
{
    int i = 0;

    fprintf(dot, "digraph BB {\n");

    list_each(code->blocks, bb)
    {
        fprintf(dot, "%d[label=\"BB (%d)\"];\n", bb->bbid, bb->bbid);
    }
    
    list_each(code->blocks, bb)
    {
         if(NULL != bb->jmp)
         {
             fprintf(dot, "bb_%d -> bb_%d ;\n",bb->bbid, bb->jmp->bbid);
         }
         if(NULL != bb->next)
         {
             fprintf(dot, "bb_%d -> bb_%d ;\n", bb->bbid, bb->next->bbid);
         }

         i = 0;
         while(bb->pred[i] != NULL)
         {
             fprintf(out, " bb_%d -> bb_%d", bb->bbid, bb->pred[i]->bbid);
             ++i;
         }
    }
    
    fprintf(dot, "}\n");
}

void
tac_free(tac_t *t)
{
    if(t->out) free(t->out);
    if(t->in) free(t->in);
    free(t);
}
