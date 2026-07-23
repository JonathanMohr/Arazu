#include "section.h"

#include "arazu/core/object/section.h"
#include "symbol.h"
#include "relocation.h"

#include <context.h>

Arazu_Size Arazu_Object_Section_Size(void)
{
    return sizeof(Arazu_Object_Section);
}

Arazu_Bool Arazu_Object_Section_Create(
    Arazu_Object_Section* out,
    const Arazu_Context* ctx,

    Arazu_String name,
    Arazu_uValue align,

    Arazu_Object_Section_Flags flags,
    Arazu_Object_Section_Type type
)
{
    if (!out) return ARAZU_FALSE;

    (void)ctx;

    out->size = 0;
    out->relocationCount = 0;
    out->symbolCount = 0;

    out->align = align;

    out->flags = flags;

    out->bufferCapacity = 0;
    out->relocationCapacity = 0;
    out->symbolCapacity = 0;

    out->buffer = ARAZU_NULL;
    out->relocations = ARAZU_NULL;
    out->symbols = ARAZU_NULL;

    out->name = name;

    out->type = type;

    return ARAZU_TRUE;
}

void Arazu_Object_Section_Destroy(const Arazu_Context* ctx, Arazu_Object_Section* section)
{
    if (section->buffer != ARAZU_NULL)
    {
        ctx->allocator.free(&ctx->allocator, section->buffer);
        section->buffer = ARAZU_NULL;
    }

    if (section->relocations != ARAZU_NULL)
    {
        for (Arazu_uValue i = 0; i < section->relocationCount; i++)
            Arazu_Object_Relocation_Destroy(ctx, &section->relocations[i]);
        ctx->allocator.free(&ctx->allocator, section->relocations);
        section->relocations = ARAZU_NULL;
    }

    if (section->symbols != ARAZU_NULL)
    {
        for (Arazu_uValue i = 0; i < section->symbolCount; i++)
            Arazu_Object_Symbol_Destroy(ctx, &section->symbols[i]);
        ctx->allocator.free(&ctx->allocator, section->symbols);
        section->symbols = ARAZU_NULL;
    }
}


Arazu_Bool Arazu_Object_Section_PushByte(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_u8 byte)
{
    if (Arazu_Object_Section_ReserveBufferSize(ctx, section, section->size + 1) != ARAZU_TRUE)
        return ARAZU_FALSE;

    section->buffer[section->size++] = byte;

    return ARAZU_TRUE;
}

Arazu_Bool Arazu_Object_Section_AddRelocation(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Object_Relocation* relocation)
{
    if (Arazu_Object_Section_ReserveRelocationCount(ctx, section, section->relocationCount + 1) != ARAZU_TRUE)
        return ARAZU_FALSE;

    if (Arazu_Object_Relocation_Copy(&section->relocations[section->relocationCount++], ctx, relocation) != ARAZU_TRUE)
        return ARAZU_FALSE;

    return ARAZU_TRUE;
}

Arazu_Bool Arazu_Object_Section_AddSymbol(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Object_Symbol* symbol)
{
    if (Arazu_Object_Section_ReserveSymbolCount(ctx, section, section->symbolCapacity + 1) != ARAZU_TRUE)
        return ARAZU_FALSE;

    if (Arazu_Object_Symbol_Copy(&section->symbols[section->symbolCount++], ctx, symbol) != ARAZU_TRUE)
        return ARAZU_FALSE;

    return ARAZU_TRUE;
}


Arazu_Bool Arazu_Object_Section_ReserveBufferSize(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Size size)
{
    if (section->bufferCapacity >= size)
        return ARAZU_TRUE;

    Arazu_u8* newBuffer = ctx->allocator.allocate(&ctx->allocator, size);
    if (!newBuffer)
        return ARAZU_FALSE;

    for (Arazu_uValue i = 0; i < section->size; i++)
        newBuffer[i] = section->buffer[i];

    ctx->allocator.free(&ctx->allocator, section->buffer);
    section->buffer = newBuffer;

    return ARAZU_TRUE;
}

Arazu_Bool Arazu_Object_Section_ReserveRelocationCount(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Size count)
{
    if (section->relocationCapacity >= count)
        return ARAZU_TRUE;

    Arazu_Object_Relocation* newRelocations = ctx->allocator.allocate(&ctx->allocator, count);
    if (!newRelocations)
        return ARAZU_FALSE;

    for (Arazu_uValue i = 0; i < section->relocationCount; i++)
        newRelocations[i] = section->relocations[i];

    ctx->allocator.free(&ctx->allocator, section->relocations);
    section->relocations = newRelocations;

    return ARAZU_TRUE;
}

Arazu_Bool Arazu_Object_Section_ReserveSymbolCount(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Size count)
{
    if (section->symbolCapacity >= count)
        return ARAZU_TRUE;

    Arazu_Object_Symbol* newSymbols = ctx->allocator.allocate(&ctx->allocator, count);
    if (!newSymbols)
        return ARAZU_FALSE;

    for (Arazu_uValue i = 0; i < section->symbolCount; i++)
        newSymbols[i] = section->symbols[i];

    ctx->allocator.free(&ctx->allocator, section->symbols);
    section->symbols = newSymbols;

    return ARAZU_TRUE;
}


Arazu_String Arazu_Object_Section_GetName(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->name;
}

Arazu_uValue Arazu_Object_Section_GetAlign(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->align;
}

Arazu_Object_Section_Flags Arazu_Object_Section_GetFlags(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->flags;
}

Arazu_Object_Section_Type Arazu_Object_Section_GetType(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->type;
}


Arazu_uValue Arazu_Object_Section_GetSize(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->size;
}

const Arazu_u8* Arazu_Object_Section_GetBuffer(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->buffer;
}

Arazu_uValue Arazu_Object_Section_GetRelocationCount(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->relocationCount;
}

const Arazu_Object_Relocation* Arazu_Object_Section_GetRelocations(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->relocations;
}

Arazu_uValue Arazu_Object_Section_GetSymbolCount(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->symbolCount;
}

const Arazu_Object_Symbol* Arazu_Object_Section_GetSymbols(const Arazu_Context* ctx, const Arazu_Object_Section* section)
{
    (void)ctx;
    return section->symbols;
}
