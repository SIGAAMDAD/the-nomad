#ifndef _N_PLATFORM_
#define _N_PLATFORM_

#pragma once

#ifdef _WIN32

#define DLL_EXT ".dll"
#define PATH_SEP '\\'
#define PATH_SEP_FOREIGN '/'
#define GDR_DECL __cdecl
#define GDR_NEWLINE "\r\n"
#define GDR_INLINE __inline

#if defined(_MSVC_VER) && _MSVC_VER >= 1400
#define COMPILER_STRING "msvc"
#elif defined(__MINGW32__)
#define COMPILER_STRING "mingw64"
#elif defined(__MINGW64__)
#define COMPILER_STRING "mingw32"
#else
#error "unsupported windows compiler"
#endif

#ifdef _WIN64
#define OS_STRING "win64 " COMPILER_STRING
#else
#define OS_STRING "win32 " COMPILER_STRING
#endif

#if defined(_M_IX86)
#define ARCH_STRING "x86"
#define GDR_LITTLE_ENDIAN
#endif

#if defined(_M_AMD64)
#define ARCH_STRING "x86_64"
#define GDR_LITTLE_ENDIAN
#endif

#if defined(_M_ARM64)
#define ARCH_STRING "arm64"
#define GDR_LITTLE_ENDIAN
#endif

#else // !defined _WIN32

// common unix platform stuff

#define DLL_EXT ".so"
#define PATH_SEP '/'
#define PATH_SEP_FOREIGN '\\'
#define GDR_DECL
#define GDR_NEWLINE "\n"

#if defined(__i386__)
#define ARCH_STRING "i386"
#define GDR_LITTLE_ENDIAN
#endif

#if defined(__x86_64__) || defined(__amd64__)
#define ARCH_STRING "x86-64"
#define GDR_LITTLE_ENDIAN
#endif

#if defined(__arm__)
#define ARCH_STRING "arm"
#define GDR_LITTLE_ENDIAN
#endif

#if defined(__aarch64__)
#define ARCH_STRING "aarch64"
#define GDR_LITTLE_ENDIAN
#endif

#endif

#if defined(__ANDROID__)

#define OS_STRING "android"
#error "android isn't yet supported"

#endif

#if defined(__linux__)

#include <endian.h>

#define OS_STRING "linux"
#define GDR_INLINE inline

#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#include <sys/types.h>
#include <machine/endian.h>

#ifdef __FreeBSD__
#define OS_STRING "freebsd"
#elif defined(__OpenBSD__)
#define OS_STRING "openbsd"
#elif defined(__NetBSD__)
#define OS_STRING "netbsd"
#endif

#define GDR_INLINE inline

#if BYTE_ORDER == BIG_ENDIAN
#define GDR_BIG_ENDIAN
#else
#define GDR_LITTLE_ENDIAN
#endif

#endif

#ifdef __APPLE__

#define OS_STRING "macos"
#define GDR_INLINE inline
#undef DLL_EXT
#define DLL_EXT ".dylib"

#endif

#ifdef Q3_VM

#define OS_STRING "q3vm"
#define GDR_INLINE

#define ARCH_STRING "bytecode"
#define GDR_LITTLE_ENDIAN

#undef DLL_EXT
#define DLL_EXT ".qvm"

#endif

#if !defined( OS_STRING )
#error "Operating system not supported"
#endif

#if !defined( ARCH_STRING )
#error "Architecture not supported"
#endif

#ifndef GDR_INLINE
#error "GDR_INLINE not defined"
#endif

#ifndef PATH_SEP
#error "PATH_SEP not defined"
#endif

#ifndef PATH_SEP_FOREIGN
#error "PATH_SEP_FOREIGN not defined"
#endif

#ifndef DLL_EXT
#error "DLL_EXT not defined"
#endif

#if defined( GDR_BIG_ENDIAN ) && defined( GDR_LITTLE_ENDIAN )
#error "Endianness defined as both big and little"
#elif defined( GDR_BIG_ENDIAN )

#define CopyLittleShort(dest, src) CopyShortSwap(dest, src)
#define CopyLittleLong(dest, src) CopyLongSwap(dest, src)
#define LittleShort(x) ShortSwap(x)
#define LittleLong(x) LongSwap(x)
#define LittleFloat(x) FloatSwap(&x)
#define BigShort
#define BigLong
#define BigFloat

#elif defined( GDR_LITTLE_ENDIAN )

#define CopyLittleShort(dest, src) Com_Memcpy(dest, src, 2)
#define CopyLittleLong(dest, src) Com_Memcpy(dest, src, 4)
#define LittleShort
#define LittleLong
#define LittleFloat
#define BigShort(x) ShortSwap(x)
#define BigLong(x) LongSwap(x)
#define BigFloat(x) FloatSwap(&x)

#else
#error "Endianness not defined"
#endif

#endif