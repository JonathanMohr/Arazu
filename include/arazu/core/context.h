#ifndef ARAZU_CORE_CONTEXT_H
#define ARAZU_CORE_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "detail.h"
#include "types.h"

typedef struct Arazu_Allocator
{
    /** Allocates memory of a size in bytes aligned to ARAZU_FUNDAMENTAL_ALIGNMENT and returns its pointer or ARAZU_NULL on failure */
    void* (*allocate)(const struct Arazu_Allocator* allocator, Arazu_Size size);

    /** Allocates memory of a size in bytes aligned to alignment and returns its pointer or ARAZU_NULL on failure */
    void* (*allocateAligned)(const struct Arazu_Allocator* allocator, Arazu_Size size, Arazu_Size alignment);

    /** Frees memory which was allocated */
    void (*free)(const struct Arazu_Allocator* allocator, void* ptr);

    /** Returns pointer to a memory area of newSize and takes care of the previous pointer or ARAZU_NULL on failure */
    void* (*reallocate)(const struct Arazu_Allocator* allocator, void* ptr, Arazu_Size newSize);

    void* userdata;

} Arazu_Allocator;

typedef struct Arazu_Context Arazu_Context;

/** Creates context for Arazu functions, shallow copies arguments and returns ARAZU_NULL on failure */
ARAZU_DETAIL_API Arazu_Context* Arazu_Context_Create(const Arazu_Allocator* allocator);

/** Destroys and frees a created Arazu context */
ARAZU_DETAIL_API void Arazu_Context_Destroy(Arazu_Context* ctx);

#ifdef __cplusplus
}
#endif

#endif
