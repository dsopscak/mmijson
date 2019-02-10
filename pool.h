//  Pool.h
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)
//
//  A simple memory pool allocator, for efficiently allocating small,
//  fixed-sized pieces of memory.  Based on an exapmple from Stroustrup.

#ifndef __POOL_H__
#define __POOL_H__

#ifndef FNS_sys_types_h
#define FNS_sys_types_h
#include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* This is supposed to be conserned with alignment, hopefully this
 * value makes sense... not sure what the significance of substracting
 * 16 is 
 */
#define POOL_CHUNK_SIZE (1024*8-16)

/* NOTICE: Do not depend on the composition of these structs, they can
 * change at any time.  Any use of these structures, apart from the
 * below declared functions, is UNSUPPORTED.
 */
struct PoolLink
    {
    struct PoolLink *next;
    };

struct PoolChunk
    {
    struct PoolChunk *next;
    char mem[POOL_CHUNK_SIZE];
    };

typedef struct
    {
    struct PoolChunk *chunks;
    size_t esize;
    struct PoolLink *head;
    }
    Pool;


int PoolInit(Pool *target, size_t size);
    /* Must be called exactly once before a Pool object may be used.
     * size is the the size of the fixed allocations this pool will be
     * used for.  Calling this more than once on an object that hasn't
     * been destroyed will result in lost memory.  Returns 0 on
     * success, non-zero otherwise.  The pool object is undefined if
     * this isn't successful. */

void PoolDestroy(Pool *target);
    /* Releases all resources held by the pool.  All allocations made
     * using this pool are rendered unusable by this call.  The pool
     * itself is undefined until another call to init. */

void *PoolAlloc(Pool *target);
    /* Calling this on an uninitialized Pool object is undefined.
     * Returns a pointer to a new allocation or NULL on failure. The
     * contents of the memory are undefined. */

void PoolFree(Pool *target, void *p);
    /* Calling this on an uninitialized Pool object is undefined.
     * Releases a previously allocated element for re-use.  Calling
     * this on anything other than a value returned from PoolAlloc
     * called on the same pool, is undefined. */


#ifdef __cpluscpus
}
#endif

#endif /* ifndef __POOL_H__ */
