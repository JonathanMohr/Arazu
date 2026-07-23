#ifndef ARAZU_LCORE_OBJECT_OBJECT_H
#define ARAZU_LCORE_OBJECT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/object/object.h>

struct Arazu_Object
{
    Arazu_uValue sectionCount;
    Arazu_uValue symbolCount;

    Arazu_Size sectionCapacity;
    Arazu_Size symbolCapacity;

    Arazu_Object_Section* sections;
    Arazu_Object_Symbol* symbols; // absolute and undefined symbols

    Arazu_String architecture; // just alphanumerical (all lowercase) + '_'
    Arazu_String abi; // just alphanumerical (all lowercase) + '_'
    Arazu_u16 bitMode;
};

#ifdef __cplusplus
}
#endif

#endif
