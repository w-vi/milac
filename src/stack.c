#include <stdlib.h>
#include "stack.h"

#ifdef STACK_DEBUG
void stack_print(stackt_t *st, stack_walk_f print)
{
    ASSERT(st != NULL);
    ASSERT(print != NULL);

    for (int i = 0; i < st->cursize; ++i)
    {
        print(st->stack[i]);
    }
}
#endif //STACK_DEBUG

stackt_t * 
stack_init(stackt_t *st, unsigned int init_size)
{
    ASSERT(st == NULL);

    if (init_size < 10)
        init_size = 10;

    if ((st = (stackt_t *)malloc(sizeof(stackt_t))) == NULL)
        return NULL;

    st->size = init_size;
    st->cursize = 0;

    if ((st->stack = (void **)malloc(init_size * sizeof(void *))) == NULL)
    {
        free(st);
        return NULL;
    }

    memset(st->stack, 0, init_size * sizeof(void *));

    return st;
}

void 
stack_destroy(stackt_t *st)
{
    ASSERT(st != NULL);
	
    free(st->stack);
    free(st);
    st = NULL;
}

void * 
stack_peek(stackt_t *st)
{
    ASSERT(st != NULL);

    if (0 == st->cursize)
        return NULL;

    return st->stack[st->cursize - 1];
}

int
stack_not_empty(stackt_t *st)
{
    ASSERT(st != NULL);

    return st->cursize;
}


void * 
stack_push(stackt_t *st, void *data)
{

    ASSERT(st!= NULL);

    if (st->cursize == st->size - 1)
    {
        if ((st->stack = (void **)realloc(st->stack, (st->size * 2) * sizeof(void *))) == NULL)
            return NULL;
		
        st->size *= 2;
    }

    st->stack[st->cursize++] = data;
    return st->stack[st->cursize - 1];
}

void *
stack_pop(stackt_t *st)
{
    void *ret = NULL;

    ASSERT(st != NULL);

    if (0 == st->cursize)
        return NULL;

    ret = st->stack[--st->cursize];
    st->stack[st->cursize] = NULL;

    return ret;
}

unsigned int 
stack_size(stackt_t *st)
{
    ASSERT(st != NULL);
    return st->cursize;
}

#ifdef TEST
#include <stdio.h>
int 
main(void)
{
    stackt_t *st = NULL;

    char *junk[] = {
        "1 The first",
        "2 The second data",
        "3 The third data",
        "4 The fourth data",
        "5 The fifth datum",
        "6 The sixth piece of data",
        "7 The seventh piece of data",
        "8 The more data",
        "9 Last data"
    };

    int i;
    void *j;

    if ((st = stack_init(st, 7)) == NULL)
    {
        printf("Error init stack\n");
        exit(1);
    }

    for (i = 1; i < 10; ++i)
    {
        if ((stack_push(st, junk[i - 1])) == NULL)
        {
            stack_destroy(st);
            printf("Error adding %d item\n", i);
            exit(1);
        }
    }

    printf("Current size of stack is : %d\n", stack_size(st));

    for (i = 0; i < 10; ++i)
    {
        j = stack_pop(st);
        if (NULL == j)
            break;
        printf("%s is poped item.\n", (char *)j);
        j = stack_peek(st);
        if (NULL == j)
            break;
        printf("%s is first item.\n", (char *)j);
    }
    printf("Done.\n");
      
    stack_destroy(st);

    return 0;
}
#endif /* TEST */
