#ifndef STACK_H_
#define STACK_H_  1

#include "common.h"

BEGIN_C_DECLS

#undef stack_t

typedef struct stack_st
{
    uint32_t size;
    uint32_t cursize;
    void **stack;	
} stackt_t;

/* initialize stack */
stackt_t * stack_init(stackt_t *st, unsigned int init_size);

/* destroy stack */
void stack_destroy(stackt_t *st);

/* get top item */
void * stack_peek(stackt_t *st);

/* push new item on stack */
void * stack_push(stackt_t *st, void *data);

/* remove and return top item  */
void * stack_pop(stackt_t *st);
 
/* returns number of elements in stack */
uint32_t stack_size(stackt_t *st);

int stack_not_empty(stackt_t *st);


/*
 * DEBUGGING HELPERS
 */

#if STACK_DEBUG
/* walk stack items function pointer type */
typedef void (*stack_walk_f)(void *);
/* print all items in a stack from bottom to top */
void stack_print(stackt_t *st, stack_walk_f print);
#endif //STACK_DEBUG

END_C_DECLS

#endif /* STACK_H_ */
