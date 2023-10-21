#ifndef _BG_LIB_
#define _BG_LIB_

#pragma once

// safeguard
#ifndef Q3_VM
    #error Never include this in engine builds
#endif

#define NULL ((void *)0)
#define offsetof(type,member) ((size_t)&(((type*)0)->(member)))

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

// same as one found in math.h
#define NAN (0.0f / 0.0f)
// from dhwem3
#define INFINITY 1e30f

typedef char* va_list;
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) (ap = (va_list)0)

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

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
void sscanf(const char *buffer, const char *fmt, ...);

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
