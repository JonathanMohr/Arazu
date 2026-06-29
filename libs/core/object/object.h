#ifndef ARAZU_LCORE_OBJECT_OBJECT_H
#define ARAZU_LCORE_OBJECT_OBJECT_H

#include "arazu/core/context.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/object/object.h>

typedef Arazu_u16 Arazu_Object_Relocation_Type;
#define ARAZU_OBJECT_RELOCATION_TYPE_ABSOLUTE    ((Arazu_Object_Relocation_Type)0)
#define ARAZU_OBJECT_RELOCATION_TYPE_PC_RELATIVE ((Arazu_Object_Relocation_Type)1)

struct Arazu_Object_Relocation
{
    Arazu_u64 offsetInSection;
    Arazu_i64 addend;

    Arazu_String* symbol;

    Arazu_u16 size;
    Arazu_Object_Relocation_Type type;
    Arazu_Bool isSigned;

    Arazu_Bool littleEndian; // false -> big endian
    Arazu_Bool isSection; // if symbol is a section
};

struct Arazu_Object_Symbol
{
    Arazu_u64 value;

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

typedef Arazu_u8 Arazu_Object_Section_Flags;
#define ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED  ((Arazu_Object_Section_Flags)1 << 0)
#define ARAZU_OBJECT_SECTION_FLAGS_EXECUTABLE ((Arazu_Object_Section_Flags)1 << 1)
#define ARAZU_OBJECT_SECTION_FLAGS_WRITABLE   ((Arazu_Object_Section_Flags)1 << 2)

typedef Arazu_u8 Arazu_Object_Section_Type;
#define ARAZU_OBJECT_SECTION_TYPE_INITIALIZED   ((Arazu_Object_Section_Type)0)
#define ARAZU_OBJECT_SECTION_TYPE_UNINITIALIZED ((Arazu_Object_Section_Type)1)

struct Arazu_Object_Section
{
    Arazu_u64 size;
    Arazu_u64 align;

    struct Arazu_Object_Relocation* relocations;
    struct Arazu_Object_Symbol* symbols; // symbols in section

    Arazu_u8* buffer;
    Arazu_String name;

    Arazu_Object_Section_Flags flags;
    Arazu_Object_Section_Type type;
};

struct Arazu_Object
{
    struct Arazu_Object_Section* sections;
    struct Arazu_Object_Symbol* symbols; // absolute and undefined symbols

    Arazu_String architecture; // just alphanumerical (all lowercase) + '_'
    Arazu_String bitMode; // just alphanumerical (all lowercase) + '_'
};



#ifdef __cplusplus
}
#endif

#endif
