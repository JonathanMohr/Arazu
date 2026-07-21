#ifndef ARAZU_LCORE_OBJECT_RELOCATION_H
#define ARAZU_LCORE_OBJECT_RELOCATION_H

#include "arazu/core/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/object/relocation.h>

struct Arazu_Object_Relocation
{
    Arazu_Value addend;
    Arazu_uValue offsetInSection;

    Arazu_String symbol;

    Arazu_u16 size;
    Arazu_Object_Relocation_Type type;
    Arazu_Bool isSigned;

    Arazu_Bool littleEndian; // false -> big endian
    Arazu_Bool isSection; // if symbol is a section
};

/** Returns ARAZU_FALSE on failure */
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
);

void Arazu_Object_Relocation_Destroy_In_Place(const Arazu_Context* ctx, Arazu_Object_Relocation* relocation);

#ifdef __cplusplus
}
#endif

#endif
