#include "symbol.h"

Arazu_Size Arazu_Object_Symbol_Size(void)
{
    return sizeof(Arazu_Object_Symbol);
}

Arazu_Bool Arazu_Object_Symbol_Create(
    Arazu_Object_Symbol* out,
    const Arazu_Context* ctx,

    Arazu_uValue value,

    Arazu_String name,

    Arazu_String sectionName,
    Arazu_Object_Symbol_State state,

    Arazu_Object_Symbol_Visibility visibility,

    Arazu_Bool hasDebugInformation,
    // Only used when hasDebugInformation is ARAZU_TRUE
    Arazu_u64 line, // 0-based
    Arazu_u64 column, // 0-based
    Arazu_String file
)
{
    if (!out) return ARAZU_FALSE;
    
    (void)ctx;

    out->value = value;
    
    if (hasDebugInformation)
    {
        out->line = line;
        out->column = column;
        out->file = file;
    }

    out->name = name;

    out->sectionName = sectionName;
    out->state = state;

    out->visibility = visibility;

    out->hasDebugInformation = hasDebugInformation;

    return ARAZU_TRUE;
}

void Arazu_Object_Symbol_Destroy(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol)
{
    (void)ctx;
    (void)symbol;
}

Arazu_Bool Arazu_Object_Symbol_Copy(Arazu_Object_Symbol* out, const Arazu_Context* newCtx, const Arazu_Object_Symbol* original)
{
    (void)newCtx;
    *out = *original;
    return ARAZU_TRUE;
}


Arazu_uValue Arazu_Object_Symbol_GetValue(const Arazu_Context* ctx, const Arazu_Object_Symbol* symbol)
{
    (void)ctx;
    return symbol->value;
}

Arazu_String Arazu_Object_Symbol_GetName(const Arazu_Context* ctx, const Arazu_Object_Symbol* symbol)
{
    (void)ctx;
    return symbol->name;
}

Arazu_String Arazu_Object_Symbol_GetSectionName(const Arazu_Context* ctx, const Arazu_Object_Symbol* symbol)
{
    (void)ctx;
    return symbol->sectionName;
}

Arazu_Object_Symbol_State Arazu_Object_Symbol_GetState(const Arazu_Context* ctx, const Arazu_Object_Symbol* symbol)
{
    (void)ctx;
    return symbol->state;
}

Arazu_Object_Symbol_Visibility Arazu_Object_Symbol_GetVisibility(const Arazu_Context* ctx, const Arazu_Object_Symbol* symbol)
{
    (void)ctx;
    return symbol->visibility;
}

Arazu_Bool Arazu_Object_Symbol_HasDebugInformation(const Arazu_Context* ctx, const Arazu_Object_Symbol* symbol)
{
    (void)ctx;
    return symbol->hasDebugInformation;
}
