#ifndef ARAZU_CORE_DETAIL_H
#define ARAZU_CORE_DETAIL_H

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


#define ARAZU_INTERNAL_CONCAT_(a, b) a##b
#define ARAZU_INTERNAL_CONCAT(a, b) ARAZU_INTERNAL_CONCAT_(a, b)

#if defined(ARAZU_DETAIL_GCC)
    #define ARAZU_INTERNAL_UNIQUE_NAME(base) \
        _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wc2y-extensions\"") \
        ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_, name), __COUNTER__) \
        _Pragma("GCC diagnostic pop")
#elif defined(ARAZU_DETAIL_CLANG)
    #define ARAZU_INTERNAL_UNIQUE_NAME(base) \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wc2y-extensions\"") \
        ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_, name), __COUNTER__) \
        _Pragma("clang diagnostic pop")
#elif defined(ARAZU_DETAIL_MSVC)
    #define ARAZU_INTERNAL_UNIQUE_NAME(base) \
        ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_, name), __COUNTER__)
#else
    #define ARAZU_INTERNAL_UNIQUE_NAME(base) \
        ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_CONCAT(ARAZU_INTERNAL_, name), ARAZU_INTERNAL_CONCAT(__FILE__, __LINE__))
#endif


#if defined(ARAZU_DETAIL_CPP11)
    #define ARAZU_INTERNAL_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(ARAZU_DETAIL_C11)
    #define ARAZU_INTERNAL_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
    #define ARAZU_INTERNAL_STATIC_ASSERT(cond, msg) typedef char ARAZU_INTERNAL_UNIQUE_NAME(STATIC_ASSERT_FAILED)[(cond) ? 1 : -1]
#endif


#ifdef ARAZU_DETAIL_WINDOWS
    #ifdef ARAZU_BUILD
        #define ARAZU_DETAIL_API __declspec(dllexport)
    #else
        #define ARAZU_DETAIL_API __declspec(dllimport)
    #endif
#elif defined(ARAZU_DETAIL_MACOS) || defined(ARAZU_DETAIL_LINUX)
    #define ARAZU_DETAIL_API __attribute__((visibility("default")))
#endif


#ifdef __cplusplus
}
#endif

#endif
