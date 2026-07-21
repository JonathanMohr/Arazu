#ifndef ARAZU_CORE_OBJECT_SYMBOL_H
#define ARAZU_CORE_OBJECT_SYMBOL_H

#include "arazu/core/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "../context.h"

typedef Arazu_u16 Arazu_Object_Symbol_State;
#define ARAZU_OBJECT_SYMBOL_STATE_IN_SECTION ((Arazu_Object_Symbol_State)0)
#define ARAZU_OBJECT_SYMBOL_STATE_ABSOLUTE   ((Arazu_Object_Symbol_State)1)
#define ARAZU_OBJECT_SYMBOL_STATE_UNDEFINED  ((Arazu_Object_Symbol_State)2)

typedef Arazu_u16 Arazu_Object_Symbol_Visibility;
#define ARAZU_OBJECT_SYMBOL_VISIBILITY_LOCAL  ((Arazu_Object_Symbol_Visibility)0)
#define ARAZU_OBJECT_SYMBOL_VISIBILITY_GLOBAL ((Arazu_Object_Symbol_Visibility)1)

typedef struct Arazu_Object_Symbol Arazu_Object_Symbol;

/** Returns ARAZU_FALSE on failure */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Symbol_Create_In_Place(
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
);

/** Free resources of symbol that was created in-place */
ARAZU_DETAIL_API void Arazu_Object_Symbol_Destroy_In_Place(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

/** Get size of symbol struct */
ARAZU_DETAIL_API Arazu_Size Arazu_Object_Symbol_Size(void);

/** Returns ARAZU_NULL on failure */
ARAZU_DETAIL_API Arazu_Object_Symbol* Arazu_Object_Symbol_Create(
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
);

/** Free resources of symbol */
ARAZU_DETAIL_API void Arazu_Object_Symbol_Destroy(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

/** Get value */
ARAZU_DETAIL_API Arazu_uValue Arazu_Object_Symbol_GetValue(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

/** Get name */
ARAZU_DETAIL_API Arazu_String Arazu_Object_Symbol_GetName(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

/** Get section name */
ARAZU_DETAIL_API Arazu_String Arazu_Object_Symbol_GetSectionName(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

/** Get state */
ARAZU_DETAIL_API Arazu_Object_Symbol_State Arazu_Object_Symbol_GetState(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

/** Get visibility */
ARAZU_DETAIL_API Arazu_Object_Symbol_Visibility Arazu_Object_Symbol_GetVisibility(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

/** Check if it has debug information */
ARAZU_DETAIL_API Arazu_Bool Arazu_Object_Symbol_HasDebugInformation(const Arazu_Context* ctx, Arazu_Object_Symbol* symbol);

#ifdef __cplusplus
}
#endif

#endif
