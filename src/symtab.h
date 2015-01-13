 /*****************************************************
 * symbol table for the MILA compiler                *
 *****************************************************/
#ifndef SYMTAB_H_
#define SYMTAB_H_ 1

#include <common.h>

BEGIN_C_DECLS

struct tree_node_s;

typedef enum
{
    SVAR,
    SCONST,
    SLABEL
} symbol_type;

typedef enum
{
    VS_EMPTY,
    VS_INMEM,
    VS_INREG,
    VS_ASSIGNED
} var_state_t;

typedef struct symbol_s
{
    symbol_type type;
    uint32_t id;
    uint32_t lineno;
    int32_t val;
    char *name;
    void *aux;
    int reg;
    var_state_t state;
    struct tree_node_s * node;
} symbol_t;

typedef void (*st_walk_t)(void *);

int st_init(unsigned int size);

symbol_t * st_insert(const char *symb, uint32_t lineno, int has_id);

symbol_t * st_insert_const(int32_t value);

symbol_t * st_lookup(const char *symb) ;

symbol_t * st_temp_symbol(symbol_type type);

void st_rem_symbol(symbol_t *s);

void st_iterate(st_walk_t fn);

uint32_t st_get_var_count();

void print_symtab();
void print_symbol(void *symb);

END_C_DECLS

#endif /* SYMTAB_H_ */
