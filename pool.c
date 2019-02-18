//  pool.c
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)

#include "pool.h"

#include <stdlib.h>

/* This is supposed to be concerned with alignment, hopefully this
 * value makes sense... not sure what the significance of substracting
 * 16 is.
 */
#define POOL_CHUNK_SIZE (1024*8-16)

struct PoolLink
    {
    struct PoolLink *next;
    };

struct PoolChunk
    {
    struct PoolChunk *next;
    char mem[POOL_CHUNK_SIZE];
    };

struct Pool
    {
    struct PoolChunk *chunks;
    size_t esize;
    struct PoolLink *head;
    };


static int 
grow(Pool *target)
    {
    struct PoolChunk *newChunk = 
        (struct PoolChunk *)malloc(sizeof(struct PoolChunk));

    if (newChunk)
        {
        const int nelem = POOL_CHUNK_SIZE / target->esize;
        char *start = newChunk->mem;
        char *last = &start[(nelem-1)*target->esize];
        char *p;

        for (p = start; p < last; p += target->esize)
            ((struct PoolLink *)p)->next = 
                (struct PoolLink *)(p + target->esize);

        ((struct PoolLink *)last)->next = NULL;
        target->head = (struct PoolLink *)start;

        newChunk->next = target->chunks;
        target->chunks = newChunk;

        return 0; /* good */
        }
    return -1; /* bad */
    }
        

Pool *
PoolCreate(size_t size)
    {
    Pool *target = malloc(sizeof(Pool));
    target->esize = 
        size < sizeof(struct PoolLink *) ? 
            sizeof(struct PoolLink *) : size;

    target->head = NULL;
    target->chunks = NULL;

    return target;
    }


void
PoolDestroy(Pool *target)
    {
    struct PoolChunk *n = target->chunks;

    while (n)
        {
        struct PoolChunk *p = n;
        n = n->next;
        free(p);
        }
    free(target);
    }


void * 
PoolAlloc(Pool *target)
    {
    struct PoolLink *p = NULL;

    if (!target->head)
        if (grow(target))
            return NULL;

    p = target->head;
    target->head = p->next;

    return p;
    }


void
PoolFree(Pool *target, void *b)
    {
    struct PoolLink *p = (struct PoolLink *)b;
    p->next = target->head;
    target->head = p;
    }
