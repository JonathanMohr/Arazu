#ifndef ARAZU_LCORE_OBJECT_OBJECT_H
#define ARAZU_LCORE_OBJECT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/object/object.h>

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

struct Arazu_Object_Symbol
{
    Arazu_uValue value;

    Arazu_u64 line; // 0-based
    Arazu_u64 column; // 0-based
    Arazu_String file;

    Arazu_String name;
    Arazu_String sectionName;

    Arazu_Bool hasDebugInformation; // use line, column and file only if this is true
    Arazu_Bool inSection;
    Arazu_Bool defined; // only used when isSection is false, if true, symbol is absolute, else, symbol is undefined
    Arazu_Bool isGlobal;
};

struct Arazu_Object_Section
{
    Arazu_uValue relocationCount;
    Arazu_uValue symbolCount;

    Arazu_uValue align;

    Arazu_Object_Section_Flags flags;

    Arazu_Size relocationCapacity;
    Arazu_Size symbolCapacity;

    Arazu_uValue size;

    Arazu_Object_Relocation* relocations;
    Arazu_Object_Symbol* symbols; // symbols in section

    Arazu_u8* buffer;

    Arazu_String name;

    Arazu_Object_Section_Type type;
};

struct Arazu_Object
{
    Arazu_uValue sectionCount;
    Arazu_uValue symbolCount;

    Arazu_Size sectionCapacity;
    Arazu_Size symbolCapacity;

    Arazu_Object_Section* sections;
    Arazu_Object_Symbol* symbols; // absolute and undefined symbols

    Arazu_String architecture; // just alphanumerical (all lowercase) + '_'
    Arazu_u16 bitMode;
};

#ifdef __cplusplus
}
#endif

#endif
