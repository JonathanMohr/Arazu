#include "context.h"

Arazu_Context* Arazu_Context_Create(const Arazu_Allocator* allocator)
{
    Arazu_Context* context = allocator->allocate(allocator, sizeof(Arazu_Context));
    if (context == ARAZU_NULL)
    {
        return ARAZU_NULL;
    }

    context->allocator = *allocator;
    
    return context;
}

void Arazu_Context_Destroy(Arazu_Context* ctx)
{
    Arazu_Allocator copiedAllocator = ctx->allocator;
    copiedAllocator.free(&copiedAllocator, ctx);
}
