#ifndef __N_PLATFORM__
#define __N_PLATFORM__

#pragma once

#ifdef _WIN32
    #define IsPlatformLinux() 0
    #define IsPlatformPosix() 0
    #define IsPlatformOSX() 0
    #define IsPlatformPS5() 0
#elif defined(Q3_VM) || defined(__unix__)

#else
    #error "Wtf R U Compiling On?"
#endif

typedef unsigned char uint8_t;
typedef signed char int8_t;

#ifdef COMPILER_MSVC
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;
#else
    #ifdef USE_STDINT_TYPES
        #include <stdint.h>
        #include <stddef.h>
    #else
        typedef unsigned short int uint16_t;
        typedef unsigned int uint32_t;
        typedef unsigned long long int uint64_t;
        
        #ifdef PLATFORM_64BIT
            typedef unsigned long long int uintptr_t;
            typedef signed long long int intptr_t;
        #else
            typedef unsigned int uintptr_t;
            typedef signed int intptr_t;
        #endif
    #endif
#endif

#endif
