/***************************************************
* 3ac code generation functions for Mila compiler  *
***************************************************/
#ifndef AC3_H_
#define AC3_H_ 1

#include "common.h"
#include "symtab.h"
#include "list.h"

BEGIN_C_DECLS

#define MAXEDGES 3
#define MAX_PREDECESSORS 16

struct tree_node_s;

typedef enum code_op
{
    TNONE,
    TIFF,
    TJMP,
    TIN,
    TOUT,
    TEQ,
    TNEQ,
    TLT,
    TGT,
    TLE,
    TGE,
    TASSIGN,
    TADD,
    TSUB,
    TMUL,
    TDIV,
    TLABEL
} codeop_type;

struct bb_s;

typedef struct tac_flag_s
{
    int set : 1;
    int unused : 7;
} tac_flag_t;

typedef struct tac_s
{
    uint32_t id;
    tac_flag_t flags;
    codeop_type op;
    symbol_t *a1;
    symbol_t *a2;
    symbol_t *r;
    void *in;
    void *out;
    struct bb_s * bb;
} tac_t;

typedef struct tac_code_s
{
    uint32_t count;
    list(tac_t *, ins);
} tac_code_t;


typedef struct bb_flag_s
{
    int unreachable : 1;
    int unused : 7;
} bb_flag_t;

typedef struct lva_s
{
    uint32_t count;
    char *def;
    char *use;
    char *in;
    char *out;
} lva_t;

typedef struct ud_s
{
    uint32_t count;
    char *def;
    char *use;
    char *in;
    char *out;
} ud_t;

typedef struct bb_s
{
    bb_flag_t flags;
    uint32_t id;
    uint32_t bbid;
    list(tac_t *, ins);
    lva_t lva;
    void *in;
    void *out;
    struct bb_s * pred[MAX_PREDECESSORS];
    struct bb_s * next;
    struct bb_s * jmp;
} bb_t;

typedef struct bb_code_s
{
    uint32_t count;
    list(bb_t*, blocks);
} bb_code_t;

extern list(tac_t*, tac_code);

void tac_gen(struct tree_node_s *t);

tac_code_t * tac(struct tree_node_s *t);

int tac_is_binop(codeop_type op);

void tac_free(tac_t *t);

bb_code_t * bb_build(tac_code_t *code);

void bb_free(bb_t *b);

void bb_remove_predecessors(bb_code_t *code, bb_t *rem);

void bb_add_predecessor(bb_t *dst, bb_t *src);

bb_code_t * bb_compact(bb_code_t *code, int *changed);

uint32_t bb_predecessor_count(bb_t *b);

void bb_print(bb_t *b);

void print_tac_code(tac_code_t *code);

void print_tac(tac_t *ins, uint32_t i);

void print_bb(bb_code_t *code);

void print_bb_to_dot(bb_code_t *code, FILE *dot);


#define NEW_TAC(p)do{                                   \
if(NULL == (p = (tac_t*)calloc(1, sizeof(tac_t)))) {    \
    fprintf(stderr, "Memory allocation error.\n");      \
    exit(EXIT_FAILURE);                                 \
}                                                       \
list_push(tac_code, p);                                 \
} while(0)                                              \


END_C_DECLS
    
#endif /*AC3_H_ */
