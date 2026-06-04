#ifndef ARAZU_CORE_TYPES_H
#define ARAZU_CORE_TYPES_H

/*
    Arazu_* and ARAZU_* are both fully reserved for this library

    Types (types are stable on each platform, no matter the Compiler or Compiler-Version):
        Arazu_i8                    Signed 8-bit integer that can store values ranging from -127 to 127
        Arazu_u8                    Unsigned 8-bit integer that can only store values ranging from 0 to 255 and wraps around
        Arazu_i16                   Signed 16-bit integer that can store values ranging from -32767 to 32767
        Arazu_u16                   Unsigned 16-bit integer that can only store values ranging from 0 to 65535 and wraps around
        Arazu_i32                   Signed 32-bit integer that can store values ranging from -2147483647 to 2147483647
        Arazu_u32                   Unsigned 32-bit integer that can only store values ranging from 0 to 4294967295 and wraps around
        Arazu_i64                   Signed 64-bit integer that can store values ranging from -9223372036854775807 to 9223372036854775807
        Arazu_u64                   Unsigned 64-bit integer that can only store values ranging from 0 to 18446744073709551615 and wraps around

        Arazu_Value                 Same as Arazu_i64
        Arazu_uValue                Same as Arazu_u64


        Arazu_PointerSize           Maximum size a pointer can have as unsigned integer
        Arazu_Size                  Unsigned integer which can at least store every Arazu_PointerSize and size_t


        Arazu_Bool                  Type that can store at ARAZU_FALSE and ARAZU_TRUE

    Definitions: 
        ARAZU_FALSE                 Value that represents false for Arazu_Bool
        ARAZU_TRUE                  Value that represents true for Arazu_Bool

        ARAZU_NULL                  Invalid pointer address

*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__cplusplus)
    #define ARAZU_DETAIL_CPP


    #if __cplusplus >= 202302L
        #define ARAZU_DETAIL_CPP23
    #endif

    #if __cplusplus >= 202002L
        #define ARAZU_DETAIL_CPP20
    #endif

    #if __cplusplus >= 201703L
        #define ARAZU_DETAIL_CPP17
    #endif

    #if __cplusplus >= 201402L
        #define ARAZU_DETAIL_CPP14
    #endif

    #if __cplusplus >= 201103L
        #define ARAZU_DETAIL_CPP11
    #endif

    #define ARAZU_DETAIL_CPP98

#elif defined(__STDC_VERSION__)
    #define ARAZU_DETAIL_C


    #if __STDC_VERSION__ >= 202311L
        #define ARAZU_DETAIL_C23
    #endif

    #if __STDC_VERSION__ >= 201710L
        #define ARAZU_DETAIL_C17
    #endif

    #if __STDC_VERSION__ >= 201112L
        #define ARAZU_DETAIL_C11
    #endif

    #if __STDC_VERSION__ >= 199901L
        #define ARAZU_DETAIL_C99
    #endif

#else
    #error "Invalid language or C89/C90"
#endif

#if defined(__clang__)
    #define ARAZU_DETAIL_CLANG

#elif defined(_MSC_VER)
    #define ARAZU_DETAIL_MSVC

#elif defined(__GNUC__)
    #define ARAZU_DETAIL_GCC

#else
    #error "Unsupported toolchain"
#endif


#if defined(__i386__) || defined(_M_IX86)
    #define ARAZU_DETAIL_X86

#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
    #define ARAZU_DETAIL_X86_64

#elif defined(__arm__) || defined(_M_ARM)
    #define ARAZU_DETAIL_ARM32

#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARAZU_DETAIL_ARM64
    
#else
    #error "Unsupported architecture"
#endif


#if defined(_WIN32) || defined(_WIN64)
    #define ARAZU_DETAIL_WINDOWS

#elif defined(__APPLE__)
    #define ARAZU_DETAIL_MACOS

#elif defined(__linux__)
    #define ARAZU_DETAIL_LINUX

#else
    #error "Unsupported platform"
#endif

#ifdef ARAZU_DETAIL_WINDOWS
    typedef char Arazu_i8;
    typedef unsigned char Arazu_u8;
    
    typedef short Arazu_i16;
    typedef unsigned short Arazu_u16;

    typedef int Arazu_i32;
    typedef unsigned int Arazu_u32;

    typedef long long Arazu_i64;
    typedef unsigned long long Arazu_u64;

#elif defined(ARAZU_DETAIL_LINUX) || defined(ARAZU_DETAIL_MACOS)
    typedef char Arazu_i8;
    typedef unsigned char Arazu_u8;
    
    typedef short Arazu_i16;
    typedef unsigned short Arazu_u16;

    typedef int Arazu_i32;
    typedef unsigned int Arazu_u32;

    #if defined(ARAZU_DETAIL_X86) || defined(ARAZU_DETAIL_ARM32)
        typedef long long Arazu_i64;
        typedef unsigned long long Arazu_u64;
    #else
        typedef long Arazu_i64;
        typedef unsigned long Arazu_u64;
    #endif

#endif


typedef Arazu_i64 Arazu_Value;
typedef Arazu_u64 Arazu_uValue;


#if defined(ARAZU_DETAIL_X86) || defined(ARAZU_DETAIL_ARM32)
    typedef Arazu_u32 Arazu_PointerSize;
    
    #if defined(ARAZU_DETAIL_WINDOWS) || defined(ARAZU_DETAIL_LINUX) || defined(ARAZU_DETAIL_MACOS)
        typedef Arazu_u32 Arazu_Size;
    #endif
#else
    typedef Arazu_u64 Arazu_PointerSize;

    #if defined(ARAZU_DETAIL_WINDOWS) || defined(ARAZU_DETAIL_LINUX) || defined(ARAZU_DETAIL_MACOS)
        typedef Arazu_u64 Arazu_Size;
    #endif
#endif


typedef Arazu_u8 Arazu_Bool;
#define ARAZU_FALSE ((Arazu_Bool)0)
#define ARAZU_TRUE  ((Arazu_Bool)1)


#if defined(ARAZU_DETAIL_CPP11)
    #define ARAZU_NULL nullptr

#elif defined(ARAZU_DETAIL_C23)
    #define ARAZU_NULL nullptr

#elif defined(ARAZU_DETAIL_GCC) || defined(ARAZU_DETAIL_CLANG)
    #define ARAZU_NULL __null

#elif defined(ARAZU_DETAIL_CPP)
    #define ARAZU_NULL 0
#else
    #define ARAZU_NULL ((void*)0)

#endif


#define ARAZU_INTERNAL_CONCAT_(a, b) a##b
#define ARAZU_INTERNAL_CONCAT(a, b) ARAZU_INTERNAL_CONCAT_(a, b)

#if defined(ARAZU_DETAIL_GCC) || defined(ARAZU_DETAIL_CLANG) || defined(ARAZU_DETAIL_MSVC)
    #define ARAZU_INTERNAL_UNIQUE_NAME(base) ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_, name), __COUNTER__)
#else
    #define ARAZU_INTERNAL_UNIQUE_NAME(base) ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_, name), ARAZU_INTERNAL_CONCAT(__FILE__, __LINE__))
#endif

#if defined(ARAZU_DETAIL_CPP11)
    #define ARAZU_INTERNAL_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(ARAZU_DETAIL_C11)
    #define ARAZU_INTERNAL_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
    #define ARAZU_INTERNAL_STATIC_ASSERT(cond, msg) typedef char ARAZU_INTERNAL_UNIQUE_NAME(STATIC_ASSERT_FAILED)[(cond) ? 1 : -1]
#endif

#define ARAZU_INTERNAL_CHECK_SIZE(s1, s2, msg) ARAZU_INTERNAL_STATIC_ASSERT((s1) == (s2), msg)

#ifdef __cplusplus
}
#endif

ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_i8), 1, "Arazu_i8 not 1 byte and 8 bits in size");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_u8), 1, "Arazu_u8 not 1 byte and 8 bits in size");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_i16), 2, "Arazu_i16 not 2 bytes and 16 bits in size");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_u16), 2, "Arazu_u16 not 2 bytes and 16 bits in size");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_i32), 4, "Arazu_i32 not 4 bytes and 32 bits in size");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_u32), 4, "Arazu_u32 not 4 bytes and 32 bits in size");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_i64), 8, "Arazu_i64 not 8 bytes and 64 bits in size");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_u64), 8, "Arazu_u64 not 8 bytes and 64 bits in size");

ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_Value), sizeof(Arazu_i64), "Arazu_Value not the same size as Arazu_i64");
ARAZU_INTERNAL_CHECK_SIZE(sizeof(Arazu_uValue), sizeof(Arazu_u64), "Arazu_UValue not the same size as Arazu_u64");

ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_PointerSize) >= sizeof(void*), "Arazu_PointerSize smaller than void*");
ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_PointerSize) >= sizeof(int*), "Arazu_PointerSize smaller than int*");
ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_PointerSize) >= sizeof(void(*)(void)), "Arazu_PointerSize smaller than void(*)(void)");
ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_PointerSize) >= sizeof(void(*)(int)), "Arazu_PointerSize smaller than void(*)(int)");
ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_PointerSize) >= sizeof(char(*)(void)), "Arazu_PointerSize smaller than char(*)(void)");
ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_PointerSize) >= sizeof(long(*)(short)), "Arazu_PointerSize smaller than long(*)(short)");

ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_Size) >= sizeof(Arazu_PointerSize), "Arazu_Size smaller than Arazu_PointerSize");
ARAZU_INTERNAL_STATIC_ASSERT(sizeof(Arazu_Size) >= sizeof(sizeof(0)), "Arazu_Size smaller than size_t");

#endif
