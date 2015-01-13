/***************************************************
* Symbol table for Mila compiler                   *
***************************************************/
#include "global.h"
#include "util.h"
#include "code.h"

#define MAX_TEMP_NAME 128

static hasht_tab_t *symbol_tab;
static uint32_t tmp_symb_id;
static uint32_t tmp_label_id;
static uint32_t symb_id = 0;

void
free_symbol(void *symb)
{
    free(symb);
}

int
st_init(uint32_t size)
{
    if(NULL != symbol_tab)
    {
        return 0;
    }

    if((symbol_tab = hash_tab_init(symbol_tab, size, NULL, &free_symbol)) == NULL)
    {
        return -1;
    }

    tmp_symb_id = 0;
    tmp_label_id = 0;

    return 0;
}

symbol_t *
new_symbol(const char *symb, uint32_t lineno, int has_id)
{
    symbol_t *s = NULL;

    ASSERT(NULL != symb);

    if((s = (symbol_t*)malloc(sizeof(symbol_t))) == NULL)
    {
        return NULL;
    }

    s->lineno = lineno;
    if(has_id)
        s->id = symb_id++;
    else
        s->id = UINT32_MAX;
    s->name = strdup(symb);
    s->val = -1;
    s->type = SVAR;
    s->node = NULL;
    s->reg = NO_REGISTER;
    s->state = VS_EMPTY;

    return s;
}

symbol_t *
st_insert(const char *symb, uint32_t lineno, int has_id)
{
    symbol_t *s  = NULL;

    ASSERT(NULL != symb);
    
    s = hash_tab_lookup(symbol_tab, symb);
    if(NULL == s)
    {
        if(NULL != (s = new_symbol(symb, lineno, has_id)))
        {
            
            if(HASH_OK != hash_tab_insert(symbol_tab, symb, s))
            {
                free_symbol(s);
                return NULL;
            }

        }
    }

    return s;
}

symbol_t *
st_insert_const(int32_t value)
{
    symbol_t *s  = NULL;
    char symb[MAX_TEMP_NAME];

    sprintf(symb, "%d", value);

    if(NULL == (s = st_insert(symb, 0, TRUE)))
    {
        fprintf(stderr, "Memory error\n");
        return NULL;
    }
    s->val = value;
    s->type = SCONST;

    return s;
}

symbol_t *
st_temp_symbol(symbol_type type)
{
    symbol_t *s = NULL;
    char symb[MAX_TEMP_NAME];

    if(type == SLABEL)
    {
        sprintf(symb, "_L%d", ++tmp_label_id);
        if(NULL == (s = st_insert(symb, 0, FALSE)))
        {
            fprintf(stderr, "Memory error\n");
            return NULL;
        }
        s->val = tmp_label_id;
    }
    else
    {
        sprintf(symb, "_t%d", tmp_symb_id++);
        if(NULL == (s = st_insert(symb, 0, TRUE)))
        {
            fprintf(stderr, "Memory error\n");
            return NULL;
        }
    }

    s->type = type;
    return s;
}

symbol_t *
st_lookup(const char *symb)
{
    symbol_t *s = NULL;
    
    s = hash_tab_lookup(symbol_tab, symb);

    return s;
}

void
st_rem_symbol(symbol_t *s)
{
    hash_tab_remove(symbol_tab, s->name);    
}


uint32_t
st_get_var_count()
{
    return symb_id;
}

void
st_iterate(st_walk_t fn)
{
    hash_tab_iterate(symbol_tab, fn);
}


void
print_symbol(void *symb)
{
    symbol_t *s = symb; 

    ASSERT(NULL != s);

    fprintf(out,"%-14s ",s->name);

    fprintf(out,"%3d    ",s->id);

    if(s->type == SCONST)
    {
        fprintf(out,"const  ");
    }
    else if(s->type == SVAR)
    {
        fprintf(out,"var    ");
    }
    else
    {
        fprintf(out,"label  ");
    }   
        
    fprintf(out, "%-8d  ", s->val);
    fprintf(out,"%-8d  ",s->reg);
    fprintf(out,"%4d ",s->lineno);
    fprintf(out,"\n");
}

void
print_symtab()
{
    fprintf(out,"Variable Name   id   Type  Value/Loc   Register   Line Numbers\n");
    fprintf(out,"-------------  ----  ----  ---------   --------   ------------\n");
    hash_tab_iterate(symbol_tab, &print_symbol);
}

