#ifndef _BG_LIB_
#define _BG_LIB_

#pragma once

#ifdef Q3_VM // safeguard

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;

// there isn't such thing as a 64-bit integer on the qvm, so if you want to actually have a 64-bit integer, use the struct instead
// NOTE: sizeof longs on the qvm is the exact same as an int
typedef unsigned long int uint64_t;
typedef signed long int int64_t;

// the qvm is 32-bit
typedef unsigned int size_t;
typedef signed int ssize_t;
typedef unsigned int ptrdiff_t;
typedef unsigned int uintptr_t;
typedef int intptr_t;

typedef char* va_list;
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) (ap = (va_list)0)

#define CHAR_BIT 8       /* number of bits in a char */
#define SCHAR_MIN (-128) /* minimum signed char value */
#define SCHAR_MAX 127    /* maximum signed char value */
#define UCHAR_MAX 0xff   /* maximum unsigned char value */

#define SHRT_MIN (-32768)           /* minimum (signed) short value */
#define SHRT_MAX 32767              /* maximum (signed) short value */
#define USHRT_MAX 0xffff            /* maximum unsigned short value */
#define INT_MIN (-2147483647 - 1)   /* minimum (signed) int value */
#define INT_MAX 2147483647          /* maximum (signed) int value */
#define UINT_MAX 0xffffffff         /* maximum unsigned int value */
#define LONG_MIN (-2147483647L - 1) /* minimum (signed) long value */
#define LONG_MAX 2147483647L        /* maximum (signed) long value */
#define ULONG_MAX 0xffffffffUL      /* maximum unsigned long value */

#endif

#endif