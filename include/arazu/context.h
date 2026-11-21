#ifndef ARAZU_CORE_CONTEXT_H
#define ARAZU_CORE_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "detail.h"
#include "types.h"

typedef const void* Arazu_String;

typedef struct Arazu_StringPool
{
    /** Interns a null-terminated string and returns its handle, or ARAZU_NULL on failure */
    Arazu_String (*intern)(struct Arazu_StringPool* pool, const char* str);

    /** Compares two interned strings, returns ARAZU_TRUE if equal */
    Arazu_Bool (*compare)(const struct Arazu_StringPool* pool, Arazu_String a, Arazu_String b);

    /** Returns a temporarily valid const char* for the given handle, valid until the next intern call */
    const char* (*toCString)(const struct Arazu_StringPool* pool, Arazu_String str);

    /** Destroys the pool and frees all interned strings */
    void (*destroy)(struct Arazu_StringPool* pool);

    void* userdata;

} Arazu_StringPool;

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

typedef struct Arazu_Context
{
    Arazu_Allocator allocator;
    Arazu_StringPool stringPool;
} Arazu_Context;

#ifdef __cplusplus
}
#endif

#endif
