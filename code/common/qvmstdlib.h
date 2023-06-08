#ifndef _QVMSTDLIB_
#define _QVMSTDLIB_

#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846f // matches value in gcc v2 math.h
#endif

#define max(a, b) (a) > (b) ? a : b
#define min(a, b) (a) < (b) ? a : b

#define NULL 0
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

typedef char* va_list;
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) (ap = (va_list)0)

typedef int cmp_t(const void*, const void*);
void qsort(void* a, size_t n, size_t es, cmp_t* cmp);
void srand(unsigned seed);
int rand(void);

double atof(const char* string);
double _atof(const char** stringPtr);
int atoi(const char* string);
int _atoi(const char** stringPtr);

void* memcpy(void *dst, const void *src, size_t n);
void* memset(void *dst, int fill, size_t n);
void* memchr(void *ptr, int delegate, size_t n);
void* memmove(void *dst, const void *src, size_t n);
void* memccpy(void *dst, const void *src, int c, size_t n);

// string functions
char* strcpy(char *dst, const char *src);
char* strncpy(char *dst, const char *src, size_t n);
char* strstr(const char *needle, const char *haystack);
char* strcat(char *dst, const char *src);
char* strchr(const char *str, int c);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *str);

int vsprintf(char *buffer, const char *fmt, va_list argptr);

int tolower(int c);
int toupper(int c);
int abs(int n);
double fabs(double x);

#endif
