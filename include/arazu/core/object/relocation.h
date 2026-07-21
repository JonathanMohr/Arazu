#ifndef ARAZU_CORE_OBJECT_RELOCATION_H
#define ARAZU_CORE_OBJECT_RELOCATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../context.h"

typedef Arazu_u16 Arazu_Object_Relocation_Type;
#define ARAZU_OBJECT_RELOCATION_TYPE_ABSOLUTE    ((Arazu_Object_Relocation_Type)0)
#define ARAZU_OBJECT_RELOCATION_TYPE_PC_RELATIVE ((Arazu_Object_Relocation_Type)1)

typedef struct Arazu_Object_Relocation Arazu_Object_Relocation;

/** Returns ARAZU_FALSE on failure */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Relocation_Create_In_Place(
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
);

/** Free resources of relocation that was created in-place */
ARAZU_DETAIL_API void Arazu_Object_Relocation_Destroy_In_Place(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Get size of relocation struct */
ARAZU_DETAIL_API Arazu_Size Arazu_Object_Relocation_Size(void);

/** Returns ARAZU_NULL on failure */
ARAZU_DETAIL_API Arazu_Object_Relocation* Arazu_Object_Relocation_Create(
    const Arazu_Context* ctx,

    Arazu_Value addend,
    Arazu_uValue offsetInSection,

    Arazu_String symbol,

    Arazu_u16 size, // 8, 16, 24, 32, 64, ...
    Arazu_Object_Relocation_Type type,
    Arazu_Bool isSigned,

    Arazu_Bool littleEndian,
    Arazu_Bool isSection // is symbol a section
);

/** Free resources of relocation */
ARAZU_DETAIL_API void Arazu_Object_Relocation_Destroy(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Get addend */
ARAZU_DETAIL_API Arazu_Value Arazu_Object_Relocation_GetAddend(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Get offset in section */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_Relocation_GetOffsetInSection(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Get symbol */
ARAZU_DETAIL_API Arazu_String Arazu_Object_Relocation_GetSymbol(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Get size */
ARAZU_DETAIL_API Arazu_u16 Arazu_Object_Relocation_GetSize(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Get relocation type */
ARAZU_DETAIL_API Arazu_Object_Relocation_Type Arazu_Object_Relocation_GetType(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Check if relocation is signed */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Relocation_IsSigned(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Check if relocation is little endian */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Relocation_IsLittleEndian(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

/** Check if relocation symbol is a section */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Relocation_IsSymbolSection(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

#ifdef __cplusplus
}
#endif

#endif
