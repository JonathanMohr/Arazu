#ifndef ARAZU_LCORE_CONTEXT_H
#define ARAZU_LCORE_CONTEXT_H

#include <arazu/core/context.h>

struct Arazu_Context
{
    Arazu_Allocator allocator;
    Arazu_StringPool stringPool;
};

#endif
