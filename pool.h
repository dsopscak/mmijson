//  pool.h
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

typedef struct Pool Pool;

Pool *PoolCreate(size_t size);
    /* size is the the size of the fixed allocations for which this pool
     * will be used. Returns NULL on error. */

void PoolDestroy(Pool *target);
    /* Releases all resources held by the pool.  All allocations made
     * using this pool are rendered unusable by this call. */

void *PoolAlloc(Pool *target);
    /* Returns a pointer to a new allocation or NULL on failure. The
     * contents of the memory are undefined. */

void PoolFree(Pool *target, void *p);
    /* Releases a previously allocated element for re-use.  Calling
     * this on anything other than a value returned from PoolAlloc
     * called on the same pool, is undefined. */


#ifdef __cpluscpus
}
#endif

#endif /* ifndef __POOL_H__ */
