/***************************************************
* TM code generation functions                     *
***************************************************/
#ifndef GEN_H_
#define GEN_H_ 1

#include "common.h"

BEGIN_C_DECLS

typedef struct interval_s
{
    symbol_t *s;
    uint32_t usecount;
    uint32_t start;
    uint32_t end;
    uint32_t reg;
}   interval_t;

typedef struct reg_desc_s
{
    int reg;
    symbol_t *s;
} reg_desc_t;


bb_code_t * reg_alloc(bb_code_t *code);

int gencode(bb_code_t *code);

END_C_DECLS

#endif /* GEN_H_ */
