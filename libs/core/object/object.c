#include "object.h"

#include "arazu/core/object/section.h"
#include "section.h"
#include "symbol.h"
#include "relocation.h"

#include <context.h>

static Arazu_Bool isValidChar(char c)
{
    const unsigned char uc = (unsigned char)c;
    return (uc >= 'a' && uc <= 'z') || (uc >= '0' && uc <= '9') || uc == '_';
}

Arazu_Size Arazu_Object_Size(void)
{
    return sizeof(Arazu_Object);
}

Arazu_Bool Arazu_Object_Create(
    Arazu_Object* out,
    const Arazu_Context* ctx,

    Arazu_String architecture, // just alphanumerical (all lowercase) + '_'
    Arazu_String abi, // just alphanumerical (all lowercase) + '_'
    Arazu_u16 bitMode
)
{
    const char* architectureStr = ctx->stringPool.toCString(&ctx->stringPool, architecture);
    const char* abiStr = ctx->stringPool.toCString(&ctx->stringPool, abi);

    while (*architectureStr)
    {
        if (isValidChar(*architectureStr) != ARAZU_TRUE)
            return ARAZU_FALSE;
        architectureStr++;
    }

    while (*abiStr)
    {
        if (isValidChar(*abiStr) != ARAZU_TRUE)
            return ARAZU_FALSE;
        abiStr++;
    }

    out->sectionCount = 0;
    out->symbolCount = 0;

    out->sectionCapacity = 0;
    out->symbolCapacity = 0;

    out->sections = ARAZU_NULL;
    out->symbols = ARAZU_NULL;

    out->architecture = architecture;
    out->abi = abi;
    out->bitMode = bitMode;

    return ARAZU_TRUE;
}

void Arazu_Object_Destroy(const Arazu_Context* ctx, Arazu_Object* object)
{
    if (object->sections != ARAZU_NULL)
    {
        for (Arazu_uValue i = 0; i < object->sectionCount; i++)
            Arazu_Object_Section_Destroy(ctx, &object->sections[i]);
        ctx->allocator.free(&ctx->allocator, object->sections);
        object->sections = ARAZU_NULL;
    }

    if (object->symbols != ARAZU_NULL)
    {
        for (Arazu_uValue i = 0; i < object->symbolCount; i++)
            Arazu_Object_Symbol_Destroy(ctx, &object->symbols[i]);
        ctx->allocator.free(&ctx->allocator, object->symbols);
        object->symbols = ARAZU_NULL;
    }
}

Arazu_Bool Arazu_Object_Copy(Arazu_Object* out, const Arazu_Context* newCtx, const Arazu_Object* original)
{
    Arazu_Object_Section* newSections = newCtx->allocator.allocate(&newCtx->allocator, out->sectionCapacity * sizeof(Arazu_Object_Section));
    if (newSections == ARAZU_NULL)
    {
        return ARAZU_FALSE;
    }

    Arazu_Object_Symbol* newSymbols = newCtx->allocator.allocate(&newCtx->allocator, out->symbolCapacity * sizeof(Arazu_Object_Symbol));
    if (newSymbols == ARAZU_NULL)
    {
        newCtx->allocator.free(&newCtx->allocator, newSections);
        return ARAZU_FALSE;
    }

    for (Arazu_uValue i = 0; i < original->sectionCount; i++)
    {
        if (Arazu_Object_Section_Copy(&newSections[i], newCtx, &original->sections[i]) != ARAZU_TRUE)
        {
            newCtx->allocator.free(&newCtx->allocator, newSections);
            newCtx->allocator.free(&newCtx->allocator, newSymbols);
            return ARAZU_FALSE;
        }
    }

    for (Arazu_uValue i = 0; i < original->symbolCount; i++)
    {
        if (Arazu_Object_Symbol_Copy(&newSymbols[i], newCtx, &original->symbols[i]) != ARAZU_TRUE)
        {
            newCtx->allocator.free(&newCtx->allocator, newSections);
            newCtx->allocator.free(&newCtx->allocator, newSymbols);
            return ARAZU_FALSE;
        }
    }

    *out = *original;

    out->sections = newSections;
    out->symbols = newSymbols;

    return ARAZU_TRUE;
}


Arazu_Bool Arazu_Object_AddSection(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Object_Section* section)
{
    if (Arazu_Object_ReserveSectionCount(ctx, object, object->sectionCount + 1) != ARAZU_TRUE)
        return ARAZU_FALSE;

    if (Arazu_Object_Section_Copy(&object->sections[object->sectionCount++], ctx, section) != ARAZU_TRUE)
        return ARAZU_FALSE;

    return ARAZU_TRUE;
}

Arazu_Bool Arazu_Object_AddSymbol(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Object_Symbol* symbol)
{
    if (Arazu_Object_ReserveSymbolCount(ctx, object, object->symbolCount + 1) != ARAZU_TRUE)
        return ARAZU_FALSE;

    if (Arazu_Object_Symbol_Copy(&object->symbols[object->symbolCount++], ctx, symbol) != ARAZU_TRUE)
        return ARAZU_FALSE;

    return ARAZU_TRUE;
}


Arazu_Bool Arazu_Object_ReserveSectionCount(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Size count)
{
    if (object->sectionCapacity >= count)
        return ARAZU_TRUE;

    Arazu_Object_Section* newSections = ctx->allocator.allocate(&ctx->allocator, count * sizeof(Arazu_Object_Section));
    if (!newSections)
        return ARAZU_FALSE;

    for (Arazu_uValue i = 0; i < object->sectionCount; i++)
        newSections[i] = object->sections[i];

    ctx->allocator.free(&ctx->allocator, object->sections);
    object->sections = newSections;

    return ARAZU_TRUE;
}

Arazu_Bool Arazu_Object_ReserveSymbolCount(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Size count)
{
    if (object->symbolCapacity >= count)
        return ARAZU_TRUE;

    Arazu_Object_Symbol* newSymbols = ctx->allocator.allocate(&ctx->allocator, count * sizeof(Arazu_Object_Symbol));
    if (!newSymbols)
        return ARAZU_FALSE;

    for (Arazu_uValue i = 0; i < object->symbolCount; i++)
        newSymbols[i] = object->symbols[i];

    ctx->allocator.free(&ctx->allocator, object->symbols);
    object->symbols = newSymbols;

    return ARAZU_TRUE;
}


Arazu_String Arazu_Object_GetArchitecture(const Arazu_Context* ctx, const Arazu_Object* object)
{
    (void)ctx;
    return object->architecture;
}

Arazu_String Arazu_Object_GetAbi(const Arazu_Context* ctx, const Arazu_Object* object)
{
    (void)ctx;
    return object->abi;
}

Arazu_u16 Arazu_Object_GetBitMode(const Arazu_Context* ctx, const Arazu_Object* object)
{
    (void)ctx;
    return object->bitMode;
}


Arazu_uValue Arazu_Object_GetSectionCount(const Arazu_Context* ctx, const Arazu_Object* object)
{
    (void)ctx;
    return object->sectionCount;
}

const Arazu_Object_Section* Arazu_Object_GetSections(const Arazu_Context* ctx, const Arazu_Object* object)
{
    (void)ctx;
    return object->sections;
}

Arazu_uValue Arazu_Object_GetSymbolCount(const Arazu_Context* ctx, const Arazu_Object* object)
{
    (void)ctx;
    return object->symbolCount;
}

const Arazu_Object_Symbol* Arazu_Object_GetSymbols(const Arazu_Context* ctx, const Arazu_Object* object)
{
    (void)ctx;
    return object->symbols;
}
