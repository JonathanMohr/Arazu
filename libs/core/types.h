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

        Arazu_Value                 Signed integer that can store every Arazu_i64
        Arazu_uValue                Unsigned integer that can store every Arazu_u64


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
    #define LCTC_DETAIL_CPP


    #if __cplusplus >= 202302L
        #define LCTC_DETAIL_CPP23
    #endif

    #if __cplusplus >= 202002L
        #define LCTC_DETAIL_CPP20
    #endif

    #if __cplusplus >= 201703L
        #define LCTC_DETAIL_CPP17
    #endif

    #if __cplusplus >= 201402L
        #define LCTC_DETAIL_CPP14
    #endif

    #if __cplusplus >= 201103L
        #define LCTC_DETAIL_CPP11
    #endif

    #define LCTC_DETAIL_CPP98

#elif defined(__STDC_VERSION__)
    #define LCTC_DETAIL_C


    #if __STDC_VERSION__ >= 202311L
        #define LCTC_DETAIL_C23
    #endif

    #if __STDC_VERSION__ >= 201710L
        #define LCTC_DETAIL_C17
    #endif

    #if __STDC_VERSION__ >= 201112L
        #define LCTC_DETAIL_C11
    #endif

    #if __STDC_VERSION__ >= 199901L
        #define LCTC_DETAIL_C99
    #endif

#else
    #error "Invalid language or C89/C90"
#endif

#if defined(__clang__)
    #define LCTC_DETAIL_CLANG

#elif defined(_MSC_VER)
    #define LCTC_DETAIL_MSVC

#elif defined(__GNUC__)
    #define LCTC_DETAIL_GCC

#else
    #error "Unsupported toolchain"
#endif


#if defined(__i386__) || defined(_M_IX86)
    #define LCTC_DETAIL_X86

#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
    #define LCTC_DETAIL_X86_64

#elif defined(__arm__) || defined(_M_ARM)
    #define LCTC_DETAIL_ARM32

#elif defined(__aarch64__) || defined(_M_ARM64)
    #define LCTC_DETAIL_ARM64
    
#else
    #error "Unsupported architecture"
#endif


#if defined(_WIN32) || defined(_WIN64)
    #define LCTC_DETAIL_WINDOWS

#elif defined(__APPLE__)
    #define LCTC_DETAIL_MACOS

#elif defined(__linux__)
    #define LCTC_DETAIL_LINUX

#else
    #error "Unsupported platform"
#endif

#ifdef LCTC_DETAIL_WINDOWS
    typedef char LCTC_I8;
    typedef unsigned char LCTC_U8;
    #define LCTC_I8_MAX 127
    #define LCTC_I8_MIN (-LCTC_I8_MAX - 1)
    #define LCTC_U8_MAX 255

    typedef short LCTC_I16;
    typedef unsigned short LCTC_U16;
    #define LCTC_I16_MAX 32767
    #define LCTC_I16_MIN (-LCTC_I16_MAX - 1)
    #define LCTC_U16_MAX 65535

    typedef int LCTC_I32;
    typedef unsigned int LCTC_U32;
    #define LCTC_I32_MAX 2147483647
    #define LCTC_I32_MIN (-LCTC_I32_MAX - 1)
    #define LCTC_U32_MAX 4294967295U

    typedef long long LCTC_I64;
    typedef unsigned long long LCTC_U64;
    #define LCTC_I64_MAX 9223372036854775807LL
    #define LCTC_I64_MIN (-LCTC_I64_MAX - 1)
    #define LCTC_U64_MAX 18446744073709551615ULL

#elif defined(LCTC_DETAIL_LINUX) || defined(LCTC_DETAIL_MACOS)
    typedef char LCTC_I8;
    typedef unsigned char LCTC_U8;
    #define LCTC_I8_MAX 127
    #define LCTC_I8_MIN (-LCTC_I8_MAX - 1)
    #define LCTC_U8_MAX 255

    typedef short LCTC_I16;
    typedef unsigned short LCTC_U16;
    #define LCTC_I16_MAX 32767
    #define LCTC_I16_MIN (-LCTC_I16_MAX - 1)
    #define LCTC_U16_MAX 65535

    typedef int LCTC_I32;
    typedef unsigned int LCTC_U32;
    #define LCTC_I32_MAX 2147483647
    #define LCTC_I32_MIN (-LCTC_I32_MAX - 1)
    #define LCTC_U32_MAX 4294967295U

    #if defined(LCTC_DETAIL_X86) || defined(LCTC_DETAIL_ARM32)
        typedef long long LCTC_I64;
        typedef unsigned long long LCTC_U64;
        #define LCTC_I64_MAX 9223372036854775807LL
        #define LCTC_I64_MIN (-LCTC_I64_MAX - 1)
        #define LCTC_U64_MAX 18446744073709551615ULL
    #else
        typedef long LCTC_I64;
        typedef unsigned long LCTC_U64;
        #define LCTC_I64_MAX 9223372036854775807L
        #define LCTC_I64_MIN (-LCTC_I64_MAX - 1)
        #define LCTC_U64_MAX 18446744073709551615UL
    #endif

#endif


typedef LCTC_I64 LCTC_Value;
typedef LCTC_U64 LCTC_UValue;


#if defined(LCTC_DETAIL_X86) || defined(LCTC_DETAIL_ARM32)
    typedef LCTC_U32 LCTC_PointerSize;
    
    #if defined(LCTC_DETAIL_WINDOWS) || defined(LCTC_DETAIL_LINUX) || defined(LCTC_DETAIL_MACOS)
        typedef LCTC_U32 LCTC_Size;
    #endif
#else
    typedef LCTC_U64 LCTC_PointerSize;

    #if defined(LCTC_DETAIL_WINDOWS) || defined(LCTC_DETAIL_LINUX) || defined(LCTC_DETAIL_MACOS)
        typedef LCTC_U64 LCTC_Size;
    #endif
#endif


typedef char LCTC_Bool;
#define LCTC_FALSE ((LCTC_Bool)0)
#define LCTC_TRUE  ((LCTC_Bool)1)


#if defined(LCTC_DETAIL_CPP11)
    #define LCTC_NULL nullptr

#elif defined(LCTC_DETAIL_C23)
    #define LCTC_NULL nullptr

#elif defined(LCTC_DETAIL_GCC) || defined(LCTC_DETAIL_CLANG)
    #define LCTC_NULL __null

#elif defined(LCTC_DETAIL_CPP)
    #define LCTC_NULL 0
#else
    #define LCTC_NULL ((void*)0)

#endif

#define LCTC_DETAIL_CONCAT_(a, b) a##b
#define LCTC_DETAIL_CONCAT(a, b) LCTC_DETAIL_CONCAT_(a, b)

#if defined(LCTC_DETAIL_GCC) || defined(LCTC_DETAIL_CLANG) || defined(LCTC_DETAIL_MSVC)
    #define LCTC_DETAIL_UNIQUE_NAME(name) LCTC_DETAIL_CONCAT(LCTC_DETAIL_CONCAT(LCTC_INTERNAL_, name), __COUNTER__)
#else
    #define LCTC_DETAIL_UNIQUE_NAME(name) LCTC_DETAIL_CONCAT(LCTC_DETAIL_CONCAT(LCTC_INTERNAL_, name), __LINE__)
#endif

#if defined(LCTC_DETAIL_C11)
    #define LCTC_INTERNAL_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#elif defined(LCTC_DETAIL_CPP11)
    #define LCTC_INTERNAL_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
    #define LCTC_INTERNAL_STATIC_ASSERT(cond, msg) typedef char LCTC_DETAIL_UNIQUE_NAME(STATIC_ASSERT_FAILED)[(cond) ? 1 : -1]
#endif


#ifdef __cplusplus
}
#endif

LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_Value) >= sizeof(LCTC_I64), "LCTC_Value smaller than LCTC_I64");
LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_UValue) >= sizeof(LCTC_U64), "LCTC_UValue smaller than LCTC_U64");

LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_PointerSize) >= sizeof(void*), "LCTC_PointerSize smaller than void*");
LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_PointerSize) >= sizeof(int*), "LCTC_PointerSize smaller than int*");
LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_PointerSize) >= sizeof(void(*)(void)), "LCTC_PointerSize smaller than void(*)(void)");
LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_PointerSize) >= sizeof(long(*)(void)), "LCTC_PointerSize smaller than long(*)(void)");
LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_PointerSize) >= sizeof(int(*)(char)), "LCTC_PointerSize smaller than int(*)(char)");

LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_Size) >= sizeof(LCTC_PointerSize), "LCTC_Size smaller than LCTC_PointerSize");
LCTC_INTERNAL_STATIC_ASSERT(sizeof(LCTC_Size) >= sizeof(sizeof(0)), "LCTC_Size smaller than size_t");

#endif
