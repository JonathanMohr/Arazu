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

// TODO

#ifdef __cplusplus
}
#endif

#endif
