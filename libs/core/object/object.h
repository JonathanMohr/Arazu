#ifndef ARAZU_LCORE_OBJECT_OBJECT_H
#define ARAZU_LCORE_OBJECT_OBJECT_H

#include "arazu/core/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/object/object.h>

struct Arazu_Object_Symbol
{
    Arazu_u64 value;

    
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

    Arazu_u8* buffer;
    Arazu_String name;

    Arazu_Object_Section_Flags flags;
    Arazu_Object_Section_Type type;
};

struct Arazu_Object
{
    char tmp;
};



#ifdef __cplusplus
}
#endif

#endif
