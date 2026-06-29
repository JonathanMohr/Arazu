#include "context.h"

Arazu_Context* Arazu_Context_Create(const Arazu_Allocator* allocator, const Arazu_StringPool* stringPool)
{
    Arazu_Context* context = allocator->allocate(allocator, sizeof(Arazu_Context));
    if (context == ARAZU_NULL)
    {
        return ARAZU_NULL;
    }

    context->allocator = *allocator;
    context->stringPool = *stringPool;
    
    return context;
}

void Arazu_Context_Destroy(Arazu_Context* ctx)
{
    Arazu_Allocator copiedAllocator = ctx->allocator;
    copiedAllocator.free(&copiedAllocator, ctx);

    // Think about destroying the stringPool
}

ARAZU_DETAIL_API const Arazu_Allocator* Arazu_Context_GetAllocator(const Arazu_Context* ctx)
{
    return &ctx->allocator;
}
