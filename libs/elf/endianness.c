#include "endianness.h"
#include "arazu/core/types.h"

/*
    TODO: Currently they just expect two's complement, make them not just expect it but enforce it
*/

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define INTERNAL_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define INTERNAL_BIG_ENDIAN
#else
#error "Endianness not defined"
#endif

static inline Arazu_u8 swap_u8(Arazu_u8 val)
{
    return val;
}

static inline Arazu_u16 swap_u16(Arazu_u16 val)
{
    return (Arazu_u16)((val << 8) | (val >> 8));
}

static inline Arazu_u32 swap_u32(Arazu_u32 val)
{
    return ((val << 24) & 0xFF000000) |
           ((val << 8)  & 0x00FF0000) |
           ((val >> 8)  & 0x0000FF00) |
           ((val >> 24) & 0x000000FF);
}

static inline Arazu_u64 swap_u64(Arazu_u64 val)
{
    return ((val << 56) & 0xFF00000000000000ULL) |
           ((val << 40) & 0x00FF000000000000ULL) |
           ((val << 24) & 0x0000FF0000000000ULL) |
           ((val << 8)  & 0x000000FF00000000ULL) |
           ((val >> 8)  & 0x00000000FF000000ULL) |
           ((val >> 24) & 0x0000000000FF0000ULL) |
           ((val >> 40) & 0x000000000000FF00ULL) |
           ((val >> 56) & 0x00000000000000FFULL);
}

static inline Arazu_i8 swap_i8(Arazu_i8 val)
{
    return (Arazu_i8)swap_u8((Arazu_u8)val);
}

static inline Arazu_i16 swap_i16(Arazu_i16 val)
{
    return (Arazu_i16)swap_u16((Arazu_u16)val);
}

static inline Arazu_i32 swap_i32(Arazu_i32 val)
{
    return (Arazu_i32)swap_u32((Arazu_u32)val);
}

static inline Arazu_i64 swap_i64(Arazu_i64 val)
{
    return (Arazu_i64)swap_u64((Arazu_u64)val);
}

#if defined(INTERNAL_LITTLE_ENDIAN)
#define swap_le(type, value) value
#elif defined(INTERNAL_BIG_ENDIAN)
#define swap_le(type, value) swap_##type(value)
#endif

#if defined(INTERNAL_LITTLE_ENDIAN)
#define swap_be(type, value) swap_##type(value)
#elif defined(INTERNAL_BIG_ENDIAN)
#define swap_be(type, value) value
#endif

#define LittleEndian_Function(type) \
    Arazu_##type Arazu_Endianness_Convert_LittleEndian_##type(const Arazu_##type value) \
    { \
        return swap_le(type, value); \
    }

#define BigEndian_Function(type) \
    Arazu_##type Arazu_Endianness_Convert_BigEndian_##type(const Arazu_##type value) \
    { \
        return swap_be(type, value); \
    }


LittleEndian_Function(u8)
LittleEndian_Function(u16)
LittleEndian_Function(u32)
LittleEndian_Function(u64)

LittleEndian_Function(i8)
LittleEndian_Function(i16)
LittleEndian_Function(i32)
LittleEndian_Function(i64)


BigEndian_Function(u8)
BigEndian_Function(u16)
BigEndian_Function(u32)
BigEndian_Function(u64)

BigEndian_Function(i8)
BigEndian_Function(i16)
BigEndian_Function(i32)
BigEndian_Function(i64)
