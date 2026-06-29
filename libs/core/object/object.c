#include "object.h"

Arazu_Object* Arazu_Object_Create(
    const Arazu_Context* ctx,

    Arazu_uValue sectionCount,
    Arazu_uValue symbolCount,

    Arazu_String architecture,
    Arazu_u16 bitMode
)
{
    Arazu_Object* object = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), sizeof(Arazu_Object));
    if (object == ARAZU_NULL) return ARAZU_NULL;

    object->sectionCount = 0;
    object->symbolCount = 0;

    object->sectionCapacity = sectionCount;
    object->symbolCapacity = symbolCount;

    if (sectionCount > 0)
    {
        object->sections = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), sizeof(Arazu_Object_Section) * sectionCount);
        if (object->sections == ARAZU_NULL)
        {
            Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), object);
            return ARAZU_NULL;
        }
    }

    if (symbolCount > 0)
    {
        object->symbols = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), sizeof(Arazu_Object_Symbol) * symbolCount);
        if (object->symbols == ARAZU_NULL)
        {
            Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), object->sections);
            Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), object);
            return ARAZU_NULL;
        }
    }

    object->architecture = architecture;
    object->bitMode = bitMode;

    return object;
}

void Arazu_Object_Destroy(const Arazu_Context* ctx, Arazu_Object* object)
{
    if (object->sectionCapacity > 0)
    {
        for (Arazu_uValue i = 0; i < object->sectionCount; i++)
        {
            // TODO: Call section destroyer
            Arazu_Object_Section* section = &object->sections[i];

            if (section->relocationCapacity > 0)
                Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), section->relocations);
            if (section->symbolCapacity > 0)
                Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), section->symbols);
        }
        Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), object->sections);
    }

    if (object->symbolCapacity > 0)
    {
        // HACK: Not clean, should call destroyer, but symbol currently has nothing to destroy
        Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), object->symbols);
    }

    Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), object);
}


ARAZU_DETAIL_API Arazu_Bool Arazu_Object_AddSection(
    const Arazu_Context* ctx,
    Arazu_Object* object,

    Arazu_uValue relocationCount,
    Arazu_uValue symbolCount,

    Arazu_uValue align,

    Arazu_uValue expectedSize,
    Arazu_u8* buffer,

    Arazu_String name,

    Arazu_Object_Section_Flags flags,
    Arazu_Object_Section_Type type
)
{
    if (object->sectionCapacity <= object->sectionCount)
    {
        Arazu_Object_Section* newSections = Arazu_Context_GetAllocator(ctx)->reallocate(Arazu_Context_GetAllocator(ctx), object->sections, object->sectionCapacity + 1); // TODO: Very inefficient
        if (newSections == ARAZU_NULL) return ARAZU_FALSE;
        object->sections = newSections;
    }

    Arazu_Object_Section* section = &object->sections[object->sectionCount++];

    section->relocationCount = relocationCount;
    section->symbolCount = symbolCount;

    section->align = align;

    section->flags = flags;

    section->relocationCapacity = relocationCount;
    section->symbolCapacity = symbolCount;

    section->size = expectedSize;

    section->relocations = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), sizeof(Arazu_Object_Relocation) * relocationCount);
    if (section->relocations == ARAZU_NULL)
    {
        return ARAZU_FALSE;
    }

    section->symbols = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), sizeof(Arazu_Object_Symbol) * symbolCount);
    if (section->symbols == ARAZU_NULL)
    {
        Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), section->relocations);
        return ARAZU_FALSE;
    }

    section->buffer = Arazu_Context_GetAllocator(ctx)->allocate(Arazu_Context_GetAllocator(ctx), expectedSize);
    if (section->buffer == ARAZU_NULL)
    {
        Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), section->relocations);
        Arazu_Context_GetAllocator(ctx)->free(Arazu_Context_GetAllocator(ctx), section->symbols);
        return ARAZU_FALSE;
    }

    if (buffer != ARAZU_NULL)
    {
        // TODO: Actually do it in a good way
        for (Arazu_uValue i = 0; i < expectedSize; i++)
            section->buffer[i] = buffer[i];
    }

    section->name = name;

    section->type = type;

    return ARAZU_TRUE;
}

ARAZU_DETAIL_API Arazu_uValue Arazu_Object_GetSectionCount(const Arazu_Context* ctx, Arazu_Object* object)
{
    (void)ctx;
    return object->sectionCount;
}

ARAZU_DETAIL_API Arazu_Object_Section* Arazu_Object_GetSection(const Arazu_Context* ctx, Arazu_Object* object, Arazu_uValue index)
{
    (void)ctx;
    if (index >= object->sectionCount) return ARAZU_NULL;
    return &object->sections[index];
}

ARAZU_DETAIL_API const Arazu_Object_Section* Arazu_Object_GetConstSection(const Arazu_Context* ctx, const Arazu_Object* object, Arazu_uValue index)
{
    (void)ctx;
    if (index >= object->sectionCount) return ARAZU_NULL;
    return &object->sections[index];
}


ARAZU_DETAIL_API Arazu_Bool Arazu_Object_AddSymbol(
    const Arazu_Context* ctx,
    Arazu_Object* object,

    Arazu_uValue value,
    Arazu_String name,
    Arazu_Bool defined,
    Arazu_Bool isGlobal,

    Arazu_Bool hasDebugInformation,
    Arazu_uValue line,
    Arazu_uValue column,
    Arazu_String file
)
{
    if (object->symbolCapacity <= object->symbolCount)
    {
        Arazu_Object_Symbol* newSymbols = Arazu_Context_GetAllocator(ctx)->reallocate(Arazu_Context_GetAllocator(ctx), object->symbols, object->symbolCapacity + 1); // TODO: Very inefficient
        if (newSymbols == ARAZU_NULL) return ARAZU_FALSE;
        object->symbols = newSymbols;
    }

    Arazu_Object_Symbol* symbol = &object->symbols[object->symbolCount++];

    symbol->value = value;

    if (hasDebugInformation == ARAZU_TRUE)
    {
        symbol->hasDebugInformation = ARAZU_TRUE;
        symbol->line = line;
        symbol->column = column;
        symbol->file = file;
    }
    else symbol->hasDebugInformation = ARAZU_FALSE;

    symbol->name = name;
    symbol->inSection = ARAZU_FALSE;
    symbol->defined = defined;
    symbol->isGlobal = isGlobal;

    return ARAZU_TRUE;
}

ARAZU_DETAIL_API Arazu_uValue Arazu_Object_GetSymbolCount(const Arazu_Context* ctx, Arazu_Object* object)
{
    (void)ctx;
    return object->symbolCount;
}

ARAZU_DETAIL_API Arazu_Object_Symbol* Arazu_Object_GetSymbol(const Arazu_Context* ctx, Arazu_Object* object, Arazu_uValue index)
{
    (void)ctx;
    if (index >= object->symbolCount) return ARAZU_NULL;
    return &object->symbols[index];
}

ARAZU_DETAIL_API const Arazu_Object_Symbol* Arazu_Object_GetConstSymbol(const Arazu_Context* ctx, const Arazu_Object* object, Arazu_uValue index)
{
    (void)ctx;
    if (index >= object->symbolCount) return ARAZU_NULL;
    return &object->symbols[index];
}


ARAZU_DETAIL_API const Arazu_String* Arazu_Object_GetArchitecture(const Arazu_Context* ctx, Arazu_Object* object)
{
    (void)ctx;
    return &object->architecture;
}

ARAZU_DETAIL_API Arazu_u32 Arazu_Object_GetBitMode(const Arazu_Context* ctx, Arazu_Object* object)
{
    (void)ctx;
    return object->bitMode;
}
