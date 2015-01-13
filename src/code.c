/***************************************************
* TM code generation functions for Mila compiler   *
***************************************************/
#include "global.h"
#include "util.h"
#include "code.h"

#define NEW_TM(p, c)do{                                     \
if(NULL == (p = (tmins_t*)calloc(1, sizeof(tmins_t)))) {    \
    fprintf(stderr, "Memory allocation error.\n");          \
    exit(EXIT_FAILURE);                                     \
}                                                           \
list_push(tm_code, p);                                      \
if(!c) ++tm_count;                                          \
} while(0)                                                  \


list(tmins_t *, tm_code);
uint32_t tm_count;


static int emit_loc = 0 ;

static int high_emit_loc = 0;

void
emit_comment(char * c)
{
    tmins_t *tm = NULL;
    
    if(trace_code)
    {
        NEW_TM(tm, TRUE);
        tm->line = emit_loc;
        tm->type = RC;
        tm->comment = c;
    }
}

void
emit_RO(inop_t op, int r, int s, int t, char *c)
{
    tmins_t *tm = NULL;
    
    NEW_TM(tm, FALSE);
    tm->type = RO;
    tm->line = emit_loc++;
    tm->op = op;
    tm->r1 = r;
    tm->r2 = s;
    tm->r3 = t;
    tm->comment = c;

    if (high_emit_loc < emit_loc) high_emit_loc = emit_loc ;
} 

void
emit_RM(inop_t op, int r, int d, int s, char *c)
{
    tmins_t *tm = NULL;
    
    NEW_TM(tm, FALSE);
    tm->type = RM;
    tm->line = emit_loc++;
    tm->op = op;
    tm->r1 = r;
    tm->r2 = d;
    tm->r3 = s;
    tm->comment = c;
    
    if (high_emit_loc < emit_loc)  high_emit_loc = emit_loc ;
} 

int
emit_skip(int how_many)
{
    int i = emit_loc;

    emit_loc += how_many ;
    if (high_emit_loc < emit_loc)  high_emit_loc = emit_loc ;

    return i;
} 

void
emit_backup(int loc)
{
    ASSERT(loc > high_emit_loc);

    emit_loc = loc ;
} 

void
emit_restore(void)
{
    emit_loc = high_emit_loc;
}

void
emit_RM_abs(inop_t op, int r, int a, char * c)
{
    tmins_t *tm = NULL;
    
    NEW_TM(tm, FALSE);
    tm->type = RM;
    tm->line = emit_loc;
    tm->op = op;
    tm->r1 = r;
    tm->r2 = a - (emit_loc + 1);
    tm->r3 = PC;
    tm->comment = c;

    ++emit_loc;
    
    if (high_emit_loc < emit_loc) high_emit_loc = emit_loc ;
} 
