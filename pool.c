//  Pool.c
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)

#include "Pool.h"

#include <stdlib.h>

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
        

int
PoolInit(Pool *target, size_t size)
    {
    target->esize = 
        size < sizeof(struct PoolLink *) ? 
            sizeof(struct PoolLink *) : size;

    target->head = NULL;
    target->chunks = NULL;

    return 0;
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
