#ifndef ARAZU_LCORE_OBJECT_SECTION_H
#define ARAZU_LCORE_OBJECT_SECTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/object/section.h>

struct Arazu_Object_Section
{
    Arazu_uValue size;
    Arazu_uValue relocationCount;
    Arazu_uValue symbolCount;

    Arazu_uValue align;

    Arazu_Object_Section_Flags flags;

    Arazu_Size bufferCapacity;
    Arazu_Size relocationCapacity;
    Arazu_Size symbolCapacity;

    Arazu_u8* buffer;

    Arazu_Object_Relocation* relocations;
    Arazu_Object_Symbol* symbols; // symbols in this section

    Arazu_String name;

    Arazu_Object_Section_Type type;
};

#ifdef __cplusplus
}
#endif

#endif
