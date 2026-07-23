#ifndef ARAZU_CORE_OBJECT_SECTION_H
#define ARAZU_CORE_OBJECT_SECTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../context.h"
#include "relocation.h"
#include "symbol.h"

typedef Arazu_u64 Arazu_Object_Section_Flags;
#define ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED  ((Arazu_Object_Section_Flags)1 << 0)
#define ARAZU_OBJECT_SECTION_FLAGS_EXECUTABLE ((Arazu_Object_Section_Flags)1 << 1)
#define ARAZU_OBJECT_SECTION_FLAGS_WRITABLE   ((Arazu_Object_Section_Flags)1 << 2)

typedef Arazu_u16 Arazu_Object_Section_Type;
#define ARAZU_OBJECT_SECTION_TYPE_INITIALIZED   ((Arazu_Object_Section_Type)0)
#define ARAZU_OBJECT_SECTION_TYPE_UNINITIALIZED ((Arazu_Object_Section_Type)1)

typedef struct Arazu_Object_Section Arazu_Object_Section;

/** Get size of section struct */
ARAZU_DETAIL_API Arazu_Size Arazu_Object_Section_Size(void);


#ifdef __cplusplus
}
#endif

#endif
