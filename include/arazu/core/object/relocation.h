#ifndef ARAZU_CORE_OBJECT_RELOCATION_H
#define ARAZU_CORE_OBJECT_RELOCATION_H

#include "arazu/core/detail.h"
#include "arazu/core/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "../context.h"

typedef Arazu_u16 Arazu_Object_Relocation_Type;
#define ARAZU_OBJECT_RELOCATION_TYPE_ABSOLUTE    ((Arazu_Object_Relocation_Type)0)
#define ARAZU_OBJECT_RELOCATION_TYPE_PC_RELATIVE ((Arazu_Object_Relocation_Type)1)

typedef struct Arazu_Object_Relocation Arazu_Object_Relocation;

/** Get size of relocation struct */
ARAZU_DETAIL_API Arazu_Size Arazu_Object_Relocation_Size(void);

/** Returns ARAZU_NULL on failure, give hints on count of sections and non-section symbols */
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
