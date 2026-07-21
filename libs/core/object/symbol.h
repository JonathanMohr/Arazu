#ifndef ARAZU_LCORE_OBJECT_SYMBOL_H
#define ARAZU_LCORE_OBJECT_SYMBOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/core/object/symbol.h>

struct Arazu_Object_Symbol
{
    Arazu_uValue value;

    // Only used when hasDebugInformation is ARAZU_TRUE
    Arazu_u64 line; // 0-based
    Arazu_u64 column; // 0-based
    Arazu_String file;

    Arazu_String name;

    Arazu_String sectionName;
    Arazu_Object_Symbol_State state;

    Arazu_Object_Symbol_Visibility visibility;

    Arazu_Bool hasDebugInformation;
};

#ifdef __cplusplus
}
#endif

#endif
