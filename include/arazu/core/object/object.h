#ifndef ARAZU_CORE_OBJECT_OBJECT_H
#define ARAZU_CORE_OBJECT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../context.h"
#include "section.h"
#include "relocation.h"
#include "symbol.h"

typedef struct Arazu_Object Arazu_Object;

/** Returns ARAZU_NULL on failure, give hints on count of sections and non-section symbols */
ARAZU_DETAIL_API Arazu_Object* Arazu_Object_Create(
    const Arazu_Context* ctx,

    Arazu_uValue sectionCount,
    Arazu_uValue symbolCount,

    Arazu_String architecture,
    Arazu_u16 bitMode
);

/** Free resources of object */
ARAZU_DETAIL_API void Arazu_Object_Destroy(const Arazu_Context* ctx, Arazu_Object* object);


/**
    Add a section and potentially reallocate sections array in object,
    give hints on size of section and on count of relocations and symbols.
    buffer can be ARAZU_NULL and will be ignored then (expectedSize will still be used),
    else it will expect it to be of expectedSize and put that into the section
    (can still be expanded later)
    returns ARAZU_TRUE on success, ARAZU_FALSE on failure
*/
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
);

/** Get section count */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_GetSectionCount(const Arazu_Context* ctx, Arazu_Object* object);

/** Get section, returns ARAZU_NULL on failure */
ARAZU_DETAIL_API Arazu_Object_Section* Arazu_Object_GetSection(const Arazu_Context* ctx, Arazu_Object* object, Arazu_uValue index);
/** Get constant section, returns ARAZU_NULL on failure */
ARAZU_DETAIL_API const Arazu_Object_Section* Arazu_Object_GetConstSection(const Arazu_Context* ctx, const Arazu_Object* object, Arazu_uValue index);


/**
    Add a symbol and potentially reallocate symbols array in object.
    returns ARAZU_TRUE on success, ARAZU_FALSE on failure
*/
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_AddSymbol(
    const Arazu_Context* ctx,
    Arazu_Object* object,

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
);

/** Get symbol count */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_GetSymbolCount(const Arazu_Context* ctx, Arazu_Object* object);

/** Get symbol, returns ARAZU_NULL on failure */
ARAZU_DETAIL_API Arazu_Object_Symbol* Arazu_Object_GetSymbol(const Arazu_Context* ctx, Arazu_Object* object, Arazu_uValue index);
/** Get constant symbol, returns ARAZU_NULL on failure */
ARAZU_DETAIL_API const Arazu_Object_Symbol* Arazu_Object_GetConstSymbol(const Arazu_Context* ctx, const Arazu_Object* object, Arazu_uValue index);


/** Get architecture */
ARAZU_DETAIL_API const Arazu_String* Arazu_Object_GetArchitecture(const Arazu_Context* ctx, Arazu_Object* object);
/** Get bitMode */
ARAZU_DETAIL_API Arazu_u32 Arazu_Object_GetBitMode(const Arazu_Context* ctx, Arazu_Object* object);


// TODO: Setter, Deep Copy


#ifdef __cplusplus
}
#endif

#endif
