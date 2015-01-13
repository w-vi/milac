 /*****************************************************
 * hastable implementation - part of symbol table    *
 *****************************************************/
#include <stdlib.h>
#include <string.h>

#include "hashtab.h"

#define SHIFT 4

/* bit better hash function from kazlib
   see : http://users.footprints.net/~kaz/kazlib.html */
static uint32_t 
make_hash(const char *key, const uint32_t slots)
{
    static unsigned long randbox[] = {
        0x49848f1bU, 0xe6255dbaU, 0x36da5bdcU, 0x47bf94e9U,
        0x8cbcce22U, 0x559fc06aU, 0xd268f536U, 0xe10af79aU,
        0xc1af4d69U, 0x1d2917b5U, 0xec4c304dU, 0x9ee5016cU,
        0x69232f74U, 0xfead7bb3U, 0xe9089ab6U, 0xf012f6aeU,
    };

    ASSERT(key != NULL);
    
    const unsigned char *str = (unsigned char *)key;
    unsigned long acc = 0;

    while(*str)
    {
        acc ^= randbox[(*str + acc) & 0xf];
        acc = (acc << 1) | (acc >> 31);
        acc &= 0xffffffffU;
        acc ^= randbox[((*str++ >> 4) + acc) & 0xf];
        acc = (acc << 2) | (acc >> 30);
        acc &= 0xffffffffU;
    }

    return (uint32_t)(acc % slots);
}

/* simple hashing function */
/* static uint32_t  */
/* make_hash1(const char *key, const uint32_t slots) */
/* { */
/*     uint32_t hash = 0; */

/*     ASSERT(key != NULL); */
    
/*     while(key != '\0') */
/*     { */
/*         hash += *key; */
/*         ++key; */
/*     } */
    
/*     return (hash % slots); */
/* } */

hasht_tab_t * 
hash_tab_init(hasht_tab_t *tab, const uint32_t size,
              const hasht_func_t hash_fn, const hasht_free_t free_fn)
{
    ASSERT(tab == NULL && size > 0);

    if((tab = (hasht_tab_t*)malloc(sizeof(hasht_tab_t))) == NULL)
        return NULL;
        
    tab->collision_count = 0;
    tab->slots = size;

    if(hash_fn != NULL)
        tab->hash_func = hash_fn;
    else
        tab->hash_func = make_hash;

    if(free_fn != NULL)
        tab->free_func = free_fn;
    else
        tab->free_func = free;

    if((tab->tab = (hasht_data_t *)
        malloc(sizeof(hasht_data_t) * tab->slots)) == NULL)
    {
        free(tab);
        return NULL;
    }

    memset(tab->tab, 0, (sizeof(hasht_data_t) * tab->slots));
        
    return tab;
}

int32_t 
hash_tab_insert(hasht_tab_t *tab, const char *key, void *data)
{
    hasht_data_t *item = NULL;
    int32_t hash_key = -1;

    ASSERT(tab != NULL && key != NULL);

    if((hash_key = tab->hash_func(key, tab->slots)) == -1)
        return HASH_ERROR_FUNC;
        
    item = tab->tab + hash_key;

    if(item->key == hash_key)
    {
        while(item->next != NULL)
            item = item->next;

        if((item->next = (hasht_data_t *)malloc(sizeof(hasht_data_t))) == NULL)
            return HASH_ERROR_MEM;
        
        item = item->next;
        item->data = data;
        item->key = hash_key;
        item->key_txt = strdup(key);
        item->next = NULL;
                
        ++tab->collision_count;
        ++tab->count;

        return HASH_OK;
    }
    else//(item->key_txt == NULL)
    {
        item->data = data;
        item->key = hash_key;
        item->key_txt = strdup(key);
        item->next = NULL;

        ++tab->count;

        return HASH_OK;
    }
}

void * 
hash_tab_lookup(hasht_tab_t *tab, const char *key)
{
    int32_t hash_key = -1;
    hasht_data_t *item = NULL;

    ASSERT(tab != NULL && key != NULL);

    if((hash_key = tab->hash_func(key, tab->slots)) == -1)
        return NULL;
        
    item = tab->tab + hash_key;

    while(item != NULL && item->key_txt != NULL && strcmp(item->key_txt, key))
        item = item->next;
        
    if(item == NULL)
        return NULL;

    return item->key_txt != NULL ? item->data : NULL ;
}

void * 
hash_tab_remove(hasht_tab_t *tab, const char *key)
{
    int32_t hash_key = -1;
    hasht_data_t *item = NULL;
    hasht_data_t *prev = NULL;
    void *ret = NULL;

    ASSERT(tab != NULL && key != NULL);

    if((hash_key = tab->hash_func(key, tab->slots)) == -1)
        return NULL;
        
    item = tab->tab + hash_key;

    while(item != NULL && (strcmp(item->key_txt, key)) != 0)
    {
        prev = item;
        item = item->next;
    }

    if(item != NULL)
    {
        if(prev != NULL)
        {
            prev->next = item->next;
                        
            free(item->key_txt);
            ret = item->data;
            free(item);
        }
        else if(item->next != NULL)
        {
            free(item->key_txt);
            memcpy(item, item->next, sizeof(hasht_data_t));
        }
        else
        {
            item->key = 0;
            free(item->key_txt);
            item->key_txt = NULL;
            ret = item->data;
            item->data = NULL;
        }
        
        --tab->count;   
        return ret;
    }
        
    return NULL;
}

void * 
hash_tab_destroy(hasht_tab_t *tab)
{
    hasht_data_t *item = NULL;
    hasht_data_t *next = NULL;
    uint32_t i = 0;

    ASSERT(tab != NULL);

    for(i = 0; i < tab->slots; ++i)
    {
        item = tab->tab + i;

        if(item->key_txt != NULL)
        {
            next = item->next;

            while(next != NULL)
            {
                item->next = next->next;
                                
                free(next->key_txt);
                tab->free_func(next->data);
                free(next);                     
                next = item->next;
            }
                        
            free(item->key_txt);
            item->key_txt = NULL;
        }
    }

    free(tab->tab);
    free(tab);

    return NULL;
}

void
hash_tab_iterate(hasht_tab_t *tab, hasht_walk_t func)
{
    hasht_data_t *item = NULL;
    hasht_data_t *next = NULL;
    hasht_data_t *save = NULL;
    uint32_t i = 0;
    
    ASSERT(tab != NULL && func != NULL);
    
    for(i = 0; i < tab->slots; ++i)
    {
        item = tab->tab + i;
        if(item->key_txt != NULL)
        {
            next = item->next;
            func(item->data);
            
            while(next != NULL)
            {
                save = next->next;
                func(next->data);
                next = save;
            }
        }
    }
}

void
hash_tab_update(hasht_tab_t *tab, const char *key, void *data)
{
    int32_t hash_key = -1;
    hasht_data_t *item = NULL;

    ASSERT(tab != NULL && key != NULL);

    if((hash_key = tab->hash_func(key, tab->slots)) != -1)
    {
        item = tab->tab + hash_key;
        while(item != NULL && item->key_txt != NULL &&
              strcmp(item->key_txt, key) != 0)
        {
            item = item->next;
        }
        if(item != NULL) item->data = data;
    }
}

void
hash_tab_dump(hasht_tab_t *tab, FILE *f, int all)
{
    hasht_data_t *item = NULL;
    hasht_data_t *next = NULL;
    uint32_t i = 0;
    
    ASSERT(tab);
    
    fprintf(f, "----------------------------------------\n");
    fprintf(f, "slots: %d \t count: %d \t collisions: %d\n",tab->slots, tab->count, tab->collision_count);

    fprintf(f, "Pos\tKey\t Key txt \t hitcount\n");
    for(i = 0; i < tab->slots; ++i)
    {
        item = tab->tab + i;
        if(all)
        {
            fprintf(f, "%d\t%d\t%s\t%d\n", i, item->key, item->key_txt,
                    item->hit_count);
        }

        if(item->key_txt != NULL)
        {
            fprintf(f, "%d\t%d\t%s\t%d\n", i, item->key, item->key_txt,
                    item->hit_count);
            next = item->next;
            while(next != NULL)
            {
                fprintf(f, "%d\t%d\t%s\t%d\n", i, next->key, next->key_txt, next->hit_count);
                next = next->next;
            }
        }
    }
    
}
