#include "support.h"

/* NOLINTNEXTLINE(misc-use-internal-linkage) */
ARAZU_DETAIL_WEAK_FUNCTION(void*, memcpy, (void* dst, const void* src, Arazu_Support_size_t n),
{
    Arazu_u8* d = dst;
    const Arazu_u8* s = src;
    for (Arazu_Support_size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
})

/* NOLINTNEXTLINE(misc-use-internal-linkage) */
ARAZU_DETAIL_WEAK_FUNCTION(void*, memset, (void* ptr, int val, Arazu_Support_size_t n),
{
    Arazu_u8* d = ptr;
    const Arazu_u8 c = (Arazu_u8)val;
    for (Arazu_Support_size_t i = 0; i < n; i++)
        d[i] = c;
    return ptr;
})
