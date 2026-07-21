#include "relocation.h"
#include "arazu/core/object/relocation.h"

Arazu_Bool Arazu_Object_Relocation_Create_In_Place(
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

void Arazu_Object_Relocation_Destroy_In_Place(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation)
{
    (void)ctx;
    (void)relocation;
}

ARAZU_DETAIL_API Arazu_Size Arazu_Object_Relocation_Size(void)
{
    return sizeof(Arazu_Object_Relocation);
}

Arazu_Object_Relocation* Arazu_Object_Relocation_Create(
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
    Arazu_Object_Relocation* relocation = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), sizeof(Arazu_Object_Relocation));
    if (relocation == ARAZU_NULL) return ARAZU_NULL;

    if (Arazu_Object_Relocation_Create_In_Place(relocation,
        ctx,
        addend,
        offsetInSection,
        symbol,
        size,
        type,
        isSigned,
        littleEndian,
        isSection) != ARAZU_TRUE)
    {
        Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), relocation);
        return ARAZU_NULL;
    }

    return relocation;
}

void Arazu_Object_Relocation_Destroy(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation)
{
    Arazu_Object_Relocation_Destroy_In_Place(ctx, relocation);
    Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), relocation);
}
