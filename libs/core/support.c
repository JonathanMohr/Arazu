#include <arazu/core/types.h>

void* memcpy(void* dst, const void* src, Arazu_Support_size_t n) // NOLINT(misc-use-internal-linkage)
{
    Arazu_u8* d = dst;
    const Arazu_u8* s = src;
    for (Arazu_Support_size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
}
