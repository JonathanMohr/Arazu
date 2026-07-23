#include <stdio.h>

#include <arazu/core/context.h>
#include <arazu/core/object/object.h>

#include "allocator.h"
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

    printf("This is just a placeholder. Do not try to this. This will may create a output_arasm.o to test the library\n");

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
        Arazu_Context_Destroy(context);
        stringPool.destroy(&stringPool);
        return 1;
    }


    // TODO: Add sections and global symbols


    Arazu_Object_Destroy(context, object);
    Arazu_Context_Destroy(context);
    stringPool.destroy(&stringPool);
    return 0;
}
