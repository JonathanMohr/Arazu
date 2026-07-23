#ifndef ARAZU_CORE_OBJECT_SECTION_H
#define ARAZU_CORE_OBJECT_SECTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../context.h"
#include "relocation.h"
#include "symbol.h"

typedef Arazu_u64 Arazu_Object_Section_Flags;
#define ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED  ((Arazu_Object_Section_Flags)1 << 0)
#define ARAZU_OBJECT_SECTION_FLAGS_EXECUTABLE ((Arazu_Object_Section_Flags)1 << 1)
#define ARAZU_OBJECT_SECTION_FLAGS_WRITABLE   ((Arazu_Object_Section_Flags)1 << 2)

typedef Arazu_u16 Arazu_Object_Section_Type;
#define ARAZU_OBJECT_SECTION_TYPE_INITIALIZED   ((Arazu_Object_Section_Type)0)
#define ARAZU_OBJECT_SECTION_TYPE_UNINITIALIZED ((Arazu_Object_Section_Type)1)

typedef struct Arazu_Object_Section Arazu_Object_Section;

/** Get size of section struct */
ARAZU_DETAIL_API Arazu_Size Arazu_Object_Section_Size(void);

/** Returns ARAZU_FALSE on failure */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Section_Create(
    Arazu_Object_Section* out,
    const Arazu_Context* ctx,

    Arazu_String name,
    Arazu_uValue align,

    Arazu_Object_Section_Flags flags,
    Arazu_Object_Section_Type type
);

/** Free resources of section that was created in-place */
ARAZU_DETAIL_API void Arazu_Object_Section_Destroy(const Arazu_Context* ctx, Arazu_Object_Section* section);


/** Add byte to buffer, may reserve memory */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Section_PushByte(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_u8 byte);

/** Add byte to buffer, may reserve memory */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Section_AddRelocation(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Object_Relocation* relocation);

/** Add byte to buffer, may reserve memory */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Section_AddSymbol(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Object_Symbol* symbol);


/** Reserve space for the buffer */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Section_ReserveBufferSize(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Size size);

/** Reserve space for the relocations */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Section_ReserveRelocationCount(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Size count);

/** Reserve space for the symbols */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Section_ReserveSymbolCount(const Arazu_Context* ctx, Arazu_Object_Section* section, Arazu_Size count);


/** Get name */
ARAZU_DETAIL_API Arazu_String Arazu_Object_Section_GetName(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get align */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_Section_GetAlign(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get flags */
ARAZU_DETAIL_API Arazu_Object_Section_Flags Arazu_Object_Section_GetFlags(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get type */
ARAZU_DETAIL_API Arazu_Object_Section_Type Arazu_Object_Section_GetType(const Arazu_Context* ctx, const Arazu_Object_Section* section);


/** Get size */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_Section_GetSize(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get buffer */
ARAZU_DETAIL_API const Arazu_u8* Arazu_Object_Section_GetBuffer(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get relocation count */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_Section_GetRelocationCount(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get relocations */
ARAZU_DETAIL_API const Arazu_Object_Relocation* Arazu_Object_Section_GetRelocations(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get symbol count */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_Section_GetSymbolCount(const Arazu_Context* ctx, const Arazu_Object_Section* section);

/** Get symbols */
ARAZU_DETAIL_API const Arazu_Object_Symbol* Arazu_Object_Section_GetSymbols(const Arazu_Context* ctx, const Arazu_Object_Section* section);

#ifdef __cplusplus
}
#endif

#endif
