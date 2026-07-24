#include <stdio.h>

#include <arazu/core/context.h>
#include <arazu/core/object/object.h>

#include "allocator.h"
#include "arazu/core/object/section.h"
#include "arazu/core/types.h"
#include "string_pool.h"

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    Arazu_Allocator allocator = make_allocator();
    Arazu_StringPool stringPool = make_string_pool(0);

    if (stringPool.userdata == ARAZU_NULL)
    {
        fputs("Failed to create string pool\n", stderr);

        return 1;
    }
    Arazu_Context* context = Arazu_Context_Create(&allocator, &stringPool);
    if (context == ARAZU_NULL)
    {
        fputs("Failed to create Arazu context\n", stderr);

        stringPool.destroy(&stringPool);
        return 1;
    }

    Arazu_Object* object = Arazu_Context_GetAllocator(context)->allocate(Arazu_Context_GetAllocator(context), Arazu_Object_Size());
    if (object == ARAZU_NULL)
    {
        fputs("Failed to allocate memory for object\n", stderr);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_Create(object, context, stringPool.intern(&stringPool, "x86_64"), stringPool.intern(&stringPool, "systemv"), 32) != ARAZU_TRUE)
    {
        fputs("Failed to create object\n", stderr);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    if (Arazu_Object_ReserveSectionCount(context, object, 2) != ARAZU_TRUE)
    {
        fputs("Failed to reserve 2 sections in object\n", stderr);

        Arazu_Object_Destroy(context, object);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }


    // TODO: Add sections and global symbols
    Arazu_Object_Section* section = Arazu_Context_GetAllocator(context)->allocate(Arazu_Context_GetAllocator(context), Arazu_Object_Section_Size());
    if (section == ARAZU_NULL)
    {
        fputs("Failed to allocate memory for section\n", stderr);

        Arazu_Object_Destroy(context, object);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    // .text

    if (Arazu_Object_Section_Create(section, context, stringPool.intern(&stringPool, ".text"), 8, ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED | ARAZU_OBJECT_SECTION_FLAGS_EXECUTABLE, ARAZU_OBJECT_SECTION_TYPE_INITIALIZED) != ARAZU_TRUE)
    {
        fputs("Failed to create .text section\n", stderr);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);

        Arazu_Object_Destroy(context, object);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    // ..., bytes, symbols, relocations

    if (Arazu_Object_AddSection(context, object, section) != ARAZU_TRUE)
    {
        fputs("Failed to add .text section to object\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);

        Arazu_Object_Destroy(context, object);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }
    Arazu_Object_Section_Destroy(context, section);

    // .data

    if (Arazu_Object_Section_Create(section, context, stringPool.intern(&stringPool, ".data"), 8, ARAZU_OBJECT_SECTION_FLAGS_ALLOCATED | ARAZU_OBJECT_SECTION_FLAGS_EXECUTABLE, ARAZU_OBJECT_SECTION_TYPE_INITIALIZED) != ARAZU_TRUE)
    {
        fputs("Failed to create .data section\n", stderr);

        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);

        Arazu_Object_Destroy(context, object);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }

    // ..., bytes, symbols

    if (Arazu_Object_AddSection(context, object, section) != ARAZU_TRUE)
    {
        fputs("Failed to add .data section to object\n", stderr);

        Arazu_Object_Section_Destroy(context, section);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);

        Arazu_Object_Destroy(context, object);
        Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }
    Arazu_Object_Section_Destroy(context, section);

    Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), section);

    // ...

    Arazu_Object_Destroy(context, object);
    Arazu_Context_GetAllocator(context)->free(Arazu_Context_GetAllocator(context), object);

    Arazu_Context_Destroy(context);
    stringPool.destroy(&stringPool);

    printf("This is just a placeholder. Do not try to this. This will may create a output_arasm.o to test the library\n");

    return 0;
}
