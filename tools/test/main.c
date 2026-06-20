#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include <arazu/core/context.h>

static void* Allocate(const Arazu_Allocator* allocator, Arazu_Size size)
{
    (void)allocator;
    return malloc(size);
}

static void Free(const Arazu_Allocator* allocator, void* ptr)
{
    (void)allocator;
    free(ptr);
}

int main(void)
{
    Arazu_u64 test = 2;
    printf("%" PRIu64, test);

    Arazu_Allocator allocator = {0};
    allocator.allocate = Allocate;
    allocator.free = Free;

    Arazu_StringPool stringPool = {0};

    Arazu_Context* context = Arazu_Context_Create(&allocator, &stringPool);

    Arazu_Context_Destroy(context);

    return 0;
}
