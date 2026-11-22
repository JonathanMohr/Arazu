#ifndef ARAZU_CORE_OBJECT_OBJECT_H
#define ARAZU_CORE_OBJECT_OBJECT_H

#include "arazu/types.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <arazu/result.h>
#include <arazu/context.h>

#include <arazu/object/section.h>
#include <arazu/object/relocation.h>
#include <arazu/object/symbol.h>


typedef Arazu_u64 Arazu_Architecture;
#define ARAZU_ARCHITECTURE_X86    ((Arazu_Architecture)1)
#define ARAZU_ARCHITECTURE_X86_64 ((Arazu_Architecture)2)


typedef struct Arazu_Object Arazu_Object;


typedef struct Arazu_Object_Operations
{
    void (*destroy)(Arazu_Object* object);

    /** Initialize specific section data in sections array */
    Arazu_Result (*initializeSection)(Arazu_Object* object, const Arazu_Object_Section* section, Arazu_Size index);

    /** Initialize specific symbol data in symbols array */
    Arazu_Result (*initializeSymbol)(Arazu_Object* object, const Arazu_Object_Symbol* symbol, Arazu_Size index);

    Arazu_Size sectionCount;
    Arazu_Size sectionSize; // Size of section in array
    void* sections; // Array of struct with Arazu_Object_Section as first element

    Arazu_Size symbolCount;
    Arazu_Size symbolSize; // Size of symbol in array
    void* symbols; // Array of struct with Arazu_Object_Symbol as first element
} Arazu_Object_Operations;


struct Arazu_Object
{
    const Arazu_Object_Operations* operations;
    Arazu_Context* context;

    void* data;

    Arazu_Architecture architecture;
};


typedef struct Arazu_Object_Interface
{
    Arazu_Result (*createObject)(Arazu_Object* objectOut, const Arazu_Context* context, Arazu_Architecture architecture);
} Arazu_Object_Interface;


#ifdef __cplusplus
}
#endif

#endif
