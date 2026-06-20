#include <arazu/core/types.h>

// NOLINTNEXTLINE(misc-use-internal-linkage)
ARAZU_DETAIL_WEAK_FUNCTION(void*, memcpy, (void* dst, const void* src, Arazu_Support_size_t n), {
    Arazu_u8* d = dst;
    const Arazu_u8* s = src;
    for (Arazu_Support_size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
})
