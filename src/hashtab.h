/*****************************************************
 * hashtable part of symbol table for Mila Compiler  *
 *****************************************************/
#ifndef HASHTAB_H_
#define HASHTAB_H_ 1

#include <common.h>

BEGIN_C_DECLS
    
/*
 * error return values for convenice
 */
#define HASH_OK          0  /* it's ok  */
#define HASH_ERROR_MEM  -1  /* error in malloc or similar */
#define HASH_ERROR_FUNC -2  /* error calling hashing function */

/*
 * pointer type to hash function
 */
typedef uint32_t (*hasht_func_t)(const char *, const uint32_t);

/*
 *  pointer type to free function for data
 */
typedef void (*hasht_free_t)(void *);


typedef void (*hasht_walk_t)(void *);

/*
 * STRUCTURES
 */

typedef struct hasht_data_s
{
    int32_t key;
    uint32_t hit_count;
    char *key_txt;
    void *data;
    struct hasht_data_s *next;
} hasht_data_t;

typedef struct hasht_tab_s
{
    uint32_t slots;
    uint32_t count;
    uint32_t collision_count;
    hasht_func_t hash_func;
    hasht_free_t free_func;
    hasht_data_t *tab;
} hasht_tab_t;

/*
 *  FUNCTIONS 
 */

//hash table initialization
hasht_tab_t * hash_tab_init(hasht_tab_t *tab,
                            const uint32_t size, const hasht_func_t hash_fn,
                            const hasht_free_t free_fn);

//add data to hash table
int hash_tab_insert(hasht_tab_t *tab, const char *key, void *data);

//returns data asociated with key
void * hash_tab_lookup(hasht_tab_t *tab, const char *key);

//removes item from hash table
void * hash_tab_remove(hasht_tab_t *tab, const char *key);

//destroys hash table and all asociated data
void * hash_tab_destroy(hasht_tab_t *tab);

//irterate
void hash_tab_iterate(hasht_tab_t *tab, hasht_walk_t func);

//update data
void hash_tab_update(hasht_tab_t *tab, const char *key, void *data);

//debug : dump whole tab to file
void hash_tab_dump(hasht_tab_t *tab, FILE *f, int all);

END_C_DECLS

#endif /* HASHTAB_H_ */
