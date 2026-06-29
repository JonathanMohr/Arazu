#include <stdio.h>

#include "allocator.h"
#include "string_pool.h"

int main(int argc, const char* argv[])
{
    (void)argc;
    (void)argv;

    Arazu_Allocator allocator = make_allocator();
    Arazu_StringPool stringPool = make_string_pool(0);

    if (stringPool.userdata == ARAZU_NULL)
    {
        fprintf(stderr, "Failed to create string pool\n");
        return 1;
    }

    Arazu_Context* context = Arazu_Context_Create(&allocator, &stringPool);
    if (context == ARAZU_NULL)
    {
        fprintf(stderr, "Failed to create Arazu context\n");
        stringPool.destroy(&stringPool);
        return 1;
    }



    Arazu_Context_Destroy(context);
    stringPool.destroy(&stringPool);
    return 0;
}
