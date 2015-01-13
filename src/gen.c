/***************************************************
* Code generator for Mila compiler                 *
***************************************************/
#include "global.h"
#include "util.h"
#include "gen.h"
#include "optimize.h"
#include "bitset.h"
#include "code.h"
#include "stack.h"

static reg_desc_t registers[3] = { {R3, NULL}, {R4, NULL}, {R5, NULL}};

typedef struct
{
    tac_t *ins;
    tmins_t *tmins;
    int loc;
} jmp_t;

static list(jmp_t *, jmps);

#define NEW_JMP(j) do {                                 \
if(NULL == (j = (jmp_t*)calloc(1, sizeof(jmp_t)))) {    \
    fprintf(stderr, "Memory allocation error.\n");      \
    exit(EXIT_FAILURE);                                 \
}                                                       \
} while(0)                                              \


static const char *ins_name[] = {"HALT","IN","OUT","ADD","INC","DEC","SHR","SHL","SUB","MUL","DIV","LD","ST","LDA","LDC","JLT","JLE","JGT","JGE","JEQ","JNE"};

void
gen_load(symbol_t *s)
{
    if(s->type == SCONST)
    {
        emit_RM(LDC, s->reg, s->val, 0, "load const");
        return;
    }

    TRACE(15, "%s", s->name);
    ASSERT(s->state != VS_INREG);
    
    if(s->state == VS_INMEM)
    {
        emit_RM(LD, s->reg, s->val, GP, "load to reg");
    }
    
    s->state = VS_INREG;
}

void
gen_bb_loads(bb_t *bb)
{
    ASSERT(NULL != bb);
    ASSERT(NULL != bb->lva.out);
    
    for(int i = 0; i < REGISTER_SPARE; ++i)
    {
        if(registers[i].s != NULL)
        {
            if(registers[i].s->type != SCONST &&
               BITTEST(bb->lva.out, registers[i].s->id))
            {
                emit_RM(ST, registers[i].reg, registers[i].s->val,
                        GP, "bb boundary store");
            }
            
            registers[i].s->state = VS_INMEM;
            registers[i].s->reg = NO_REGISTER;
        }
    }
}

void
free_regs(uint32_t c, tac_t *ins)
{
    for(int i = 0; i < REGISTER_SPARE; ++i)
    {
        if(registers[i].s != NULL && registers[i].s->type != SCONST)
        {
            if(!BITTEST((char *)ins->out, registers[i].s->id))
            {
                TRACE(15,"%d BIT not set reg %d symbol %s", c, registers[i].reg, registers[i].s->name);
                registers[i].s->reg = NO_REGISTER;
                registers[i].s->state = VS_INMEM;
                registers[i].s = NULL;
            }
            
        }
    }
}

int
find_reg(symbol_t *s, symbol_t *ex1, symbol_t *ex2)
{
    /* find empty register - issue load */
    for(int i = 0; i < REGISTER_SPARE; ++i)
    {
        if(registers[i].s == NULL)
        {
            registers[i].s = s;
            s->reg = registers[i].reg;
            
            TRACE(15, "symbol %s assigned %d", s->name, s->reg);

            gen_load(s);
            
            return registers[i].reg;
        }
    }

    /* no empty reg - free one, issue store */
    for(int i = 0; i < REGISTER_SPARE; ++i)
    {
        if(registers[i].s != ex1 && registers[i].s != ex2)
        {
            if(registers[i].s->type != SCONST)// && registers[i].s->state != VS_INMEM)
            {
                emit_RM(ST, registers[i].reg, registers[i].s->val,
                        GP, "store from reg");
                registers[i].s->state = VS_INMEM;
            }

            registers[i].s->reg = NO_REGISTER;
            s->reg = registers[i].reg;
            registers[i].s = s;
            gen_load(s);
            
            return registers[i].reg;
        }
    }

    return -1;
}

int
get_reg(tac_t *ins, int *r1, int *r2, int *r3)
{
    ASSERT(r1);
    
    if(tac_is_binop(ins->op))
    {
        *r1 = ins->r->reg;
        if(NO_REGISTER == *r1) *r1 = find_reg(ins->r, ins->a1, ins->a2);

        *r2 = ins->a1->reg;
        if(NO_REGISTER == *r2) *r2 = find_reg(ins->a1, ins->a2, ins->r);
        
        *r3 = ins->a2->reg;
        if(NO_REGISTER == *r3) *r3 = find_reg(ins->a2, ins->a1, ins->r);

        ASSERT(*r1 != -1 && *r2 != -1 && *r3 != -1);
        return 0;
    }

    switch(ins->op)
    {
    case TIN:
    case TOUT:
        *r1 = ins->r->reg;
        if(NO_REGISTER == *r1) *r1 = find_reg(ins->r, NULL, NULL);
        break;
    
    case TIFF:
        break;
        
    case TASSIGN:
        *r1 = ins->r->reg;
        if(NO_REGISTER == *r1) *r1 = find_reg(ins->r, ins->a1, NULL);

        if(r2 != NULL)
        {
            *r2 = ins->a1->reg;
            if(NO_REGISTER == *r2) *r2 = find_reg(ins->a1, ins->r, NULL);
        }
        break;
    
    case TJMP:
        break;
        
    default:
        print_tac(ins, 88);
        ASSERT(0);
    }
    
    return 0;
}

void
gen_inc(tac_t *ins)
{
    int r1 = ins->r->reg;

    if(NO_REGISTER == r1) r1 = find_reg(ins->r, NULL, NULL);

    if(ins->op == TADD)
        emit_RO(INC, r1, 0, 0, "inc instead of add 1");
    else
        emit_RO(DEC, r1, 0, 0, "dec instead of sub 1");
}

void
gen_binop(tac_t *ins)
{
    int r1 = NO_REGISTER;
    int r2 = NO_REGISTER;
    int r3 = NO_REGISTER;

    if(ins->op == TADD || ins->op == TSUB)
    {
        if(ins->r == ins->a1 || ins->r == ins->a2)
        {
            if((ins->a1->type == SCONST && ins->a1->val == 1)
               || (ins->a2->type == SCONST && ins->a2->val == 1))
            {
                return gen_inc(ins);
            }
        }
    }
    
    get_reg(ins, &r1, &r2, &r3);

    switch(ins->op)
    {
    case TADD:
        emit_RO(ADD, r1, r2, r3, " op: + ");
        break;
    
    case TSUB:
        emit_RO(SUB, r1, r2, r3, " op: - ");
        break;
    
    case TMUL:
        emit_RO(MUL, r1, r2, r3, " op: * ");
        break;
            
    case TDIV:
        emit_RO(DIV, r1, r2, r3, " op: / ");
        break;
            
    case TEQ:
        emit_RO(SUB, r1, r2, r3, " op: == ");
        break;
        
    case TNEQ:
        emit_RO(SUB, r1, r2, r3, " op: != ");
        break;
        
    case TLT:
        emit_RO(SUB, r1, r2, r3, " op: < ");
        break;
        
    case TGT:
        emit_RO(SUB, r1, r2, r3, " op: > ");
        break;
        
    case TLE:
        emit_RO(SUB, r1, r2, r3, " op: <= ");
        break;
        
    case TGE:
        emit_RO(SUB, r1, r2, r3, " op: => ");
        break;
        
    default:
        ASSERT(0);
    }
}

void
gen_cond_jmp(tac_t *ins, tac_t *prev)
{
    jmp_t *jmp = NULL;
    int r1 = NO_REGISTER;
    int r2 = NO_REGISTER;
    int r3 = NO_REGISTER;

    get_reg(prev, &r1, &r2, &r3);

    gen_bb_loads(ins->bb);
    
    switch(prev->op)
    {
    case TEQ:
        emit_RM(JEQ, r1, r2, r3, " jmp: == 0 ");
        break;
        
    case TNEQ:
        emit_RO(JNE, r1, r2, r3, " jmp: != 0");
        break;
        
    case TLT:
        emit_RO(JGE, r1, r2, r3, " jmp: < 0");
        break;
        
    case TGT:
        emit_RO(JLE, r1, r2, r3, " jmp: > 0");
        break;
        
    case TLE:
        emit_RO(JGT, r1, r2, r3, " jmp: <= 0");
        break;
        
    case TGE:
        emit_RO(JLT, r1, r2, r3, " jmp: => 0");
        break;
        
    default:
        ASSERT(0);
    }

    NEW_JMP(jmp);
    jmp->ins = ins;
    jmp->tmins = list_back(tm_code);
    list_push(jmps, jmp);
}

void
gen_ins(tac_t * ins, tac_t *prev)
{
    int r1 = NO_REGISTER;
    int r2 = NO_REGISTER;
    jmp_t *jmp= NULL;
    int found = FALSE;

    if(tac_is_binop(ins->op))
    {
        gen_binop(ins);
        return;
    }

    switch(ins->op)
    {
    case TIN:
        get_reg(ins, &r1, NULL, NULL);
        emit_RO(IN, r1, 0, 0, "read value to reg");
        break;

    case TOUT:
        get_reg(ins, &r1, NULL, NULL);
        emit_RO(OUT, r1, 0, 0, "write: value from reg");
        break;

    case TIFF:
        gen_cond_jmp(ins, prev);
        break;
        
    case TASSIGN:
        if(ins->r != ins->a1)
        {
            if(ins->a1->type == SCONST)
            {
                get_reg(ins, &r1, NULL, NULL);
                emit_RM(LDC, r1, ins->a1->val, 0, "assign: constant");
            }
            else
            {
                get_reg(ins, &r1, &r2, NULL);
                emit_RM(LDC, r1, 0, 0, "assign : set reg to 0");
                emit_RO(ADD, r1, r2, r1, "assign : add the other reg");
            }
        }
        break;
    
    case TJMP:
        list_each(jmps, jmp)
        {
            if(jmp->ins->r == ins->r)
            {
                TRACE(15, "%p = %s", jmp, jmp->ins->r->name);
                emit_RM_abs(LDA, PC, jmp->loc, "while: jmp to top");
                found = TRUE;
            }
        }
        
        if(!found)
        {
            gen_bb_loads(ins->bb);
            emit_RM_abs(LDA, PC, 0, "if: skip else br");
            NEW_JMP(jmp);
            jmp->ins = ins;
            jmp->tmins = list_back(tm_code);
            list_push(jmps, jmp);
        }
        
        break;
        
    case TLABEL:
        list_each(jmps, jmp)
        {
            if(jmp->ins->r == ins->r)
            {
                if(jmp->tmins->op == LDA)
                {
                    TRACE(15, "found LDA target %s", ins->r->name);
                    jmp->tmins->r2 = (emit_skip(0) - 1) - jmp->tmins->line;
                }
                else
                {
                    TRACE(15, "found jmp target %s", ins->r->name);
                    jmp->tmins->r2 = (emit_skip(0) - 1) - jmp->tmins->line;
                    jmp->tmins->r3 = PC;
                }
                found = TRUE;
                break;
            }
        }

        if(!found)
        {
            NEW_JMP(jmp);
            jmp->ins = ins;
            jmp->loc = emit_skip(0);
            list_push(jmps, jmp);
            TRACE(15, "new jmp loc %d %s", jmp->loc, ins->r->name);
        }
        break;
        
    default:
        ASSERT(0);
    }
}


int
gen(bb_code_t *code)
{
    tac_t *prev = NULL;
    uint32_t c = 0;
    int loaded = FALSE;
    
    list_each(code->blocks, blk)
    {
        registers[0].s = NULL;
        registers[1].s = NULL;
        registers[2].s = NULL;

        TRACE(15, "Block id %d", blk->bbid);
        
        lva_bb(blk);

        list_each(blk->ins, ins)
        {
            ++c;
            if(ins->op == TIFF || ins->op == TJMP)
            {
                loaded = TRUE;
            }
            gen_ins(ins, prev);
            prev = ins;
            free_regs(c, ins);
        }
        if(!loaded) gen_bb_loads(blk);
        loaded = FALSE;
    }

    emit_comment("End of execution.");
    emit_RO(HALT ,0,0,0,"");

    return 0;
}

void
peephole()
{
    TRACE(5, "Peephole optimizer pass %s", "");

    list_each(tm_code, tm)
    {
        
    }
}

void
write_file()
{
    TRACE(5, "Writing exe %s", "");

    list_each(tm_code, tm)
    {
        if(tm->type == RC)
        {
            TRACE(10, "RC: * %s",tm->comment);
            if (trace_code) fprintf(tm_file,"* %s\n", tm->comment);

            continue;
        }
        
        if(tm->type == RO)
        {
            TRACE(10, "RO: %3d:  %5s(%d)  %d,%d,%d \t%s",tm->line, ins_name[tm->op],
                  tm->op, tm->r1, tm->r2, tm->r3, tm->comment);
        
            fprintf(tm_file,"%3d:  %5s  %d,%d,%d ",tm->line, ins_name[tm->op], tm->r1,
                tm->r2, tm->r3);
        }

        if(tm->type == RM)
        {
            TRACE(10, "RM: %3d:  %5s(%d)  %d,%d(%d) \t%s",tm->line, ins_name[tm->op],
                  tm->op, tm->r1, tm->r2, tm->r3, tm->comment);
        
            fprintf(tm_file,"%3d:  %5s  %d,%d,%d ",tm->line, ins_name[tm->op], tm->r1,
                tm->r2, tm->r3);
        }
        
        if(trace_code) fprintf(tm_file, "  %s", tm->comment);
        fprintf(tm_file, "\n");
    }
}


int
gencode(bb_code_t *code)
{
    TRACE(5, "Generating code %s", "");

    memset(&tm_code, 0, sizeof (tm_code));

    memset(&jmps, 0, sizeof (jmps));

    gen(code);

    peephole();

    write_file();
    
    return TRUE;
}


