#ifndef ARAZU_LCORE_OBJECT_RELOCATION_H
#define ARAZU_LCORE_OBJECT_RELOCATION_H

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

#ifdef __cplusplus
}
#endif

#endif
