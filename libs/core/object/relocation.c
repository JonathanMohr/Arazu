#include "relocation.h"

ARAZU_DETAIL_API Arazu_Size Arazu_Object_Relocation_Size(void)
{
    return sizeof(Arazu_Object_Relocation);
}

Arazu_Bool Arazu_Object_Relocation_Create(
    Arazu_Object_Relocation* out,
    const Arazu_Context* ctx,

    Arazu_Value addend,
    Arazu_uValue offsetInSection,

    Arazu_String symbol,

    Arazu_u16 size, // 8, 16, 24, 32, 64, ...
    Arazu_Object_Relocation_Type type,
    Arazu_Bool isSigned,

    Arazu_Bool littleEndian,
    Arazu_Bool isSection // is symbol a section
)
{
    if (!out) return ARAZU_FALSE;
    
    (void)ctx;

    out->addend = addend;
    out->offsetInSection = offsetInSection;

    out->symbol = symbol;

    out->size = size;
    out->type = type;
    out->isSigned = isSigned;
    out->littleEndian = littleEndian;
    out->isSection = isSection;

    return ARAZU_TRUE;
}

void Arazu_Object_Relocation_Destroy(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    (void)relocation;
}

Arazu_Bool Arazu_Object_Relocation_Copy(Arazu_Object_Relocation* out, const Arazu_Context* newCtx, const Arazu_Object_Relocation* original)
{
    (void)newCtx;
    *out = *original;
    return ARAZU_TRUE;
}


Arazu_Value Arazu_Object_Relocation_GetAddend(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->addend;
}

Arazu_uValue Arazu_Object_Relocation_GetOffsetInSection(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->offsetInSection;
}

Arazu_String Arazu_Object_Relocation_GetSymbol(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->symbol;
}

Arazu_u16 Arazu_Object_Relocation_GetSize(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->size;
}

Arazu_Object_Relocation_Type Arazu_Object_Relocation_GetType(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->type;
}

Arazu_Bool Arazu_Object_Relocation_IsSigned(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->isSigned;
}

Arazu_Bool Arazu_Object_Relocation_IsLittleEndian(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->littleEndian;
}

Arazu_Bool Arazu_Object_Relocation_IsSymbolSection(const Arazu_Context* ctx, const Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    return relocation->isSection;
}
