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

/** Get size of object struct */
ARAZU_DETAIL_API Arazu_Size Arazu_Object_Size(void);

/** Returns ARAZU_FALSE on failure */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Create(
    Arazu_Object* out,
    const Arazu_Context* ctx,

    Arazu_String architecture, // just alphanumerical (all lowercase) + '_'
    Arazu_String abi, // just alphanumerical (all lowercase) + '_'
    Arazu_u16 bitMode
);

/** Free resources of object that was created in-place */
ARAZU_DETAIL_API void Arazu_Object_Destroy(const Arazu_Context* ctx, Arazu_Object* object);

/** Copy object */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Copy(Arazu_Object* out, const Arazu_Context* newCtx, const Arazu_Object* original);


/** Add section, may reserve memory */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_AddSection(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Object_Section* section);

/** Add symbol, may reserve memory */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_AddSymbol(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Object_Symbol* symbol);


/** Reserve space for the sections */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_ReserveSectionCount(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Size count);

/** Reserve space for the symbols */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_ReserveSymbolCount(const Arazu_Context* ctx, Arazu_Object* object, Arazu_Size count);


/** Get architecture */
ARAZU_DETAIL_API Arazu_String Arazu_Object_GetArchitecture(const Arazu_Context* ctx, const Arazu_Object* object);

/** Get abi */
ARAZU_DETAIL_API Arazu_String Arazu_Object_GetAbi(const Arazu_Context* ctx, const Arazu_Object* object);

/** Get bit mode */
ARAZU_DETAIL_API Arazu_u16 Arazu_Object_GetBitMode(const Arazu_Context* ctx, const Arazu_Object* object);


/** Get section count */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_GetSectionCount(const Arazu_Context* ctx, const Arazu_Object* object);

/** Get sections */
ARAZU_DETAIL_API const Arazu_Object_Section* Arazu_Object_GetSections(const Arazu_Context* ctx, const Arazu_Object* object);

/** Get symbol count */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_GetSymbolCount(const Arazu_Context* ctx, const Arazu_Object* object);

/** Get symbols */
ARAZU_DETAIL_API const Arazu_Object_Symbol* Arazu_Object_GetSymbols(const Arazu_Context* ctx, const Arazu_Object* object);

#ifdef __cplusplus
}
#endif

#endif
