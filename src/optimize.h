/***************************************************
* Optimizations functions for Mila compiler        *
***************************************************/
#ifndef OPTIMIZE_H_
#define OPTIMIZE_H_ 1

#include <common.h>
#include "3ac.h"

BEGIN_C_DECLS

bb_code_t * const_prop(bb_code_t *code);

bb_code_t * copy_prop(bb_code_t *code);

bb_code_t * optimize(bb_code_t *code, int32_t level);

bb_code_t * lva(bb_code_t *code);

void lva_print(bb_t *bb);

int lva_bb(bb_t *bb);

int lva_delete(bb_code_t *code);

bb_t * cse(bb_t *code);




END_C_DECLS
    
#endif /* OPTIMIZE_H_ */

