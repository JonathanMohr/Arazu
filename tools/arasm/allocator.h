#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <arazu/core/context.h>

#if defined(ARAZU_DETAIL_WINDOWS)
    #include <malloc.h>
#endif
#include <stdlib.h>

static void* alloc__allocate_aligned(const Arazu_Allocator* a,
                                     Arazu_Size size,
                                     Arazu_Size alignment)
{
    (void)a;
    if (size == 0 || alignment == 0) return ARAZU_NULL;

#if defined(ARAZU_DETAIL_WINDOWS)
    return _aligned_malloc((size_t)size, (size_t)alignment);
#else
    void* ptr = ARAZU_NULL;
    if (posix_memalign(&ptr, (size_t)alignment, (size_t)size) != 0)
        return ARAZU_NULL;
    return ptr;
#endif
}

static void* alloc__allocate(const Arazu_Allocator* a, Arazu_Size size)
{
    return alloc__allocate_aligned(a, size, ARAZU_FUNDAMENTAL_ALIGNMENT);
}

static void alloc__free(const Arazu_Allocator* a, void* ptr)
{
    (void)a;
    if (ptr == ARAZU_NULL) return;
#if defined(ARAZU_DETAIL_WINDOWS)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

static void* alloc__reallocate(const Arazu_Allocator* a,
                               void* ptr,
                               Arazu_Size newSize)
{
    (void)a;
    if (newSize == 0) { alloc__free(a, ptr); return ARAZU_NULL; }
    if (ptr == ARAZU_NULL) return malloc((size_t)newSize);
    return realloc(ptr, (size_t)newSize);
}

static inline Arazu_Allocator make_allocator(void)
{
    Arazu_Allocator a;
    a.allocate        = alloc__allocate;
    a.allocateAligned = alloc__allocate_aligned;
    a.free            = alloc__free;
    a.reallocate      = alloc__reallocate;
    a.userdata        = ARAZU_NULL;
    return a;
}

#endif
