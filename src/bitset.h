/***************************************************
* Bit set functions for Mila compiler              *
***************************************************/
#ifndef BITSET_H_
#define BITSET_H_

/*
 Taken from comp.lang.c FAQ list · Question 20.8
 Example:
 
 #include <stdio.h>
 #include <string.h>

 #define MAX 10000

 int main()
 {
     char bitarray[BITNSLOTS(MAX)];
     int i, j;

     memset(bitarray, 0, BITNSLOTS(MAX));

     for(i = 2; i < MAX; i++)
     {
         if(!BITTEST(bitarray, i))
         {
             printf("%d\n", i);
             for(j = i + i; j < MAX; j += i)
             BITSET(bitarray, j);
         }
     }
     return 0;
}

*/

#include "common.h"

BEGIN_C_DECLS

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

END_C_DECLS

#endif /*_BITSET_H_ */
