#ifndef __SG_LIB__
#define __SG_LIB__

#pragma once

#ifndef NULL
    #define NULL ((void *)0)
#endif

#ifndef SGN
    #define SGN(x) (((x) >= 0) ? !!(x) : -1)
#endif

#ifndef MAX
    #define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef CLAMP
    #define CLAMP(a,b,c) MIN(MAX((a),(b)),(c))
#endif

#ifndef offsetof
    #define offsetof(type,member) ((size_t)&(((type*)0)->(member)))
#endif

// same as one found in math.h
#define NAN (0.0f / 0.0f)
// from dhwem3
#define INFINITY 1e30f

typedef char* va_list;
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) (ap = (va_list)0)

// q3vm integer types

//
// note that types such as uint64_t and int64_t
// aren't actually 64 bits, they're just here
// to prevent functions in n_shared.h from
// bitching
//

typedef unsigned char uint8_t;
typedef signed char int8_t;

typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

typedef short int int16_t;
typedef int int32_t;
typedef long int int64_t;

typedef unsigned int uintptr_t;
typedef signed int intptr_t;
typedef unsigned int ptrdiff_t;
typedef unsigned int size_t;
typedef signed int ssize_t;

typedef int cmp_t(const void *, const void *);

/* standard library replacement functions */
void *memcpy(void *dst, const void *src, size_t n);
void *memchr(void *ptr, int delegate, size_t n);
void *memset(void *dst, int fill, size_t n);
void *memmove(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);
size_t strlen(const char *str);
char *strchr(const char *str, int c);
char *strstr(const char *needle, const char *haystack);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
int tolower(int c);
int toupper(int c);
void qsort(void *a, size_t n, size_t es, cmp_t *cmp);
void srand(unsigned seed);
int rand(void);
int vsprintf(char *buffer, const char *fmt, va_list argptr);
int sscanf( const char *buffer, const char *fmt, ... );

// Math functions
double ceil( double x );
double floor( double x );
double sqrt( double x );
double sin( double x );
double cos( double x );
double atan2( double y, double x );
double tan( double x );
int abs( int n );
double fabs( double x );
double acos( double x );

// stuff for people new to c
#define new(type) (type *)SG_MemAlloc( sizeof(type) )

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
