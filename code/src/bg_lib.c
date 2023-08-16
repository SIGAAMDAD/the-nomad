#include "../sgame/sg_version.h"
#include "../engine/n_shared.h"

#ifndef Q3_VM
    #error Never include this in engine builds
#endif


void *memset(void *dst, int fill, size_t n)
{
    char *d = dst;
    while (n--) {
        *d++ = fill;
    }
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    char *d = dst;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

void *memccpy(void *dst, const void *src, int c, size_t n)
{
    char *d = dst;
    const char *s = src;
    while (n-- && *d != c) {
        *d++ = *s++;
    }
    return dst;
}

void *memchr(void *ptr, int delegate, size_t n)
{
    char *p = ptr;
    while (n--) {
        if (*p++ == delegate) {
            return (void *)p;
        }
    }
    return ptr;
}

void *memmove(void *dst, const void *src, size_t n)
{
    char *d = dst;
    const char *s = src;
    if (d > s) {
        while (n--) {
            *d-- = *s--;
        }
    }
    else {
        while (n--) {
            *d++ = *s++;
        }
    }
    return dst;
}

size_t strlen(const char *str)
{
    const char* s;

    s = str;
    while (*s) {
        s++;
    }
    return (size_t)s - (size_t)str;
}

char* strcat(char *dst, const char *src)
{
    char *d = dst;
    const char *s = src;
    while (*d) {
        d++;
    }
    while (*src) {
        *d++ = *src++;
    }
    *d = 0;
    return dst;
}

char* strrchr(const char *str, int c)
{
    const char *found, *p;

    c = (unsigned char)c;

    if (!c)
        return strchr(str, 0);
    
    found = NULL;
    while ((p = strchr(str, c)) != NULL) {
        found = p;
        str = p + 1;
    }
    return (char *)found;
}

char* strchr(const char* str, int c)
{
    const char *s = str;
    while (*s) {
        if (*s == c) {
            return (char*)s;
        }
        s++;
    }
    return (char*)NULL;
}

char* strstr(const char* needle, const char* haystack)
{
    const char *str1 = needle;
    const char *str2 = haystack;
    while (*str1) {
        int i;
        for (i = 0; str2[i]; i++) {
            if (str1[i] != str2[i]) {
                break;
            }
        }
        if (!str2[i]) {
            return (char*)str1;
        }
        str1++;
    }
    return (char*)NULL;
}

int strcmp(const char* s1, const char* s2)
{
    const char *str1 = s1;
    const char *str2 = s2;
    while (*str1 == *str2 && *str1 && *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    const char *str1 = s1;
    const char *str2 = s2;
    while (*str1 == *str2 && *str1 && *str2 && n--) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

char* strncpy(char *dst, const char *src, size_t n)
{
    char *d = dst;
    const char *s = src;
    while (*s != '\0' && n--) {
        *d++ = *s++;
    }
    *d = 0;
    return dst;
}

char* strcpy(char *dst, const char *src)
{
    char *d = dst;
    const char *s = src;
    while (*s != '\0') {
        *d++ = *s++;
    }
    *d = 0;
    return dst;
}

int tolower(int c)
{
    if (c >= 'A' && c <= 'Z') {
        c += 'a' - 'A';
    }
    return c;
}

int toupper(int c)
{
    if (c >= 'a' && c <= 'z') {
        c += 'A' - 'a';
    }
    return c;
}

static char* med3(char*, char*, char*, cmp_t*);
static void  swapfunc(char*, char*, int, int);

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n)                                        \
    {                                                                          \
        long           i  = (n) / sizeof(TYPE);                                \
        register TYPE* pi = (TYPE*)(parmi);                                    \
        register TYPE* pj = (TYPE*)(parmj);                                    \
        do                                                                     \
        {                                                                      \
            register TYPE t = *pi;                                             \
            *pi++           = *pj;                                             \
            *pj++           = t;                                               \
        } while (--i > 0);                                                     \
    }

#define SWAPINIT(a, es)                                                        \
    swaptype = ((char*)a - (char*)0) % sizeof(long) || es % sizeof(long)       \
                   ? 2                                                         \
                   : es == sizeof(long) ? 0 : 1;

static void swapfunc(a, b, n, swaptype) char *a, *b;
int         n, swaptype;
{
    if (swaptype <= 1)
        swapcode(long, a, b, n) else swapcode(char, a, b, n)
}

#define swap(a, b)                                                             \
    if (swaptype == 0)                                                         \
    {                                                                          \
        long t      = *(long*)(a);                                             \
        *(long*)(a) = *(long*)(b);                                             \
        *(long*)(b) = t;                                                       \
    }                                                                          \
    else                                                                       \
        swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n)                                                       \
    if ((n) > 0)                                                               \
    swapfunc(a, b, n, swaptype)

static char *med3(a, b, c, cmp) char *a, *b, *c;
cmp_t*       cmp;
{
    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
}

void   qsort(a, n, es, cmp) void* a;
size_t n, es;
cmp_t* cmp;
{
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
    int   d, r, swaptype, swap_cnt;

loop:
    SWAPINIT(a, es);
    swap_cnt = 0;
    if (n < 7)
    {
        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
                swap(pl, pl - es);
        return;
    }
    pm = (char*)a + (n / 2) * es;
    if (n > 7)
    {
        pl = a;
        pn = (char*)a + (n - 1) * es;
        if (n > 40)
        {
            d  = (n / 8) * es;
            pl = med3(pl, pl + d, pl + 2 * d, cmp);
            pm = med3(pm - d, pm, pm + d, cmp);
            pn = med3(pn - 2 * d, pn - d, pn, cmp);
        }
        pm = med3(pl, pm, pn, cmp);
    }
    swap(a, pm);
    pa = pb = (char*)a + es;

    pc = pd = (char*)a + (n - 1) * es;
    for (;;)
    {
        while (pb <= pc && (r = cmp(pb, a)) <= 0)
        {
            if (r == 0)
            {
                swap_cnt = 1;
                swap(pa, pb);
                pa += es;
            }
            pb += es;
        }
        while (pb <= pc && (r = cmp(pc, a)) >= 0)
        {
            if (r == 0)
            {
                swap_cnt = 1;
                swap(pc, pd);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc)
            break;
        swap(pb, pc);
        swap_cnt = 1;
        pb += es;
        pc -= es;
    }
    if (swap_cnt == 0)
    { /* Switch to insertion sort */
        for (pm = (char*)a + es; pm < (char*)a + n * es; pm += es)
            for (pl = pm; pl > (char*)a && cmp(pl - es, pl) > 0; pl -= es)
                swap(pl, pl - es);
        return;
    }

    pn = (char*)a + n * es;
    r  = min(pa - (char*)a, pb - pa);
    vecswap(a, pb - r, r);
    r = min(pd - pc, pn - pd - es);
    vecswap(pb, pn - r, r);
    if ((r = pb - pa) > es)
        qsort(a, r / es, es, cmp);
    if ((r = pd - pc) > es)
    {
        /* Iterate rather than recurse to save stack space */
        a = pn - r;
        n = r / es;
        goto loop;
    }
    /*      qsort(pn - r, r / es, es, cmp);*/
}


static int randSeed = 0;

void srand(unsigned seed)
{
    randSeed = seed;
}

int rand(void)
{
    randSeed = (69069 * randSeed + 1);
    return randSeed & 0x7fff;
}

double atof(const char* string)
{
    float sign;
    float value;
    int   c;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
        {
            return 0;
        }
        string++;
    }

    // check sign
    switch (*string)
    {
    case '+':
        string++;
        sign = 1;
        break;
    case '-':
        string++;
        sign = -1;
        break;
    default:
        sign = 1;
        break;
    }

    // read digits
    value = 0;
    c     = string[0];
    if (c != '.')
    {
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
            {
                break;
            }
            c -= '0';
            value = value * 10 + c;
        } while (1);
    }
    else
    {
        string++;
    }

    // check for decimal point
    if (c == '.')
    {
        double fraction;

        fraction = 0.1;
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
            {
                break;
            }
            c -= '0';
            value += c * fraction;
            fraction *= 0.1;
        } while (1);
    }

    // not handling 10e10 notation...

    return value * sign;
}

double _atof(const char** stringPtr)
{
    const char* string;
    float       sign;
    float       value;
    int         c = '0'; // bk001211 - uninitialized use possible

    string = *stringPtr;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
        {
            *stringPtr = string;
            return 0;
        }
        string++;
    }

    // check sign
    switch (*string)
    {
    case '+':
        string++;
        sign = 1;
        break;
    case '-':
        string++;
        sign = -1;
        break;
    default:
        sign = 1;
        break;
    }

    // read digits
    value = 0;
    if (string[0] != '.')
    {
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
            {
                break;
            }
            c -= '0';
            value = value * 10 + c;
        } while (1);
    }

    // check for decimal point
    if (c == '.')
    {
        double fraction;

        fraction = 0.1;
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
            {
                break;
            }
            c -= '0';
            value += c * fraction;
            fraction *= 0.1;
        } while (1);
    }

    // not handling 10e10 notation...
    *stringPtr = string;

    return value * sign;
}

int atoi(const char* string)
{
    int sign;
    int value;
    int c;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
        {
            return 0;
        }
        string++;
    }

    // check sign
    switch (*string)
    {
    case '+':
        string++;
        sign = 1;
        break;
    case '-':
        string++;
        sign = -1;
        break;
    default:
        sign = 1;
        break;
    }

    // read digits
    value = 0;
    do
    {
        c = *string++;
        if (c < '0' || c > '9')
        {
            break;
        }
        c -= '0';
        value = value * 10 + c;
    } while (1);

    // not handling 10e10 notation...

    return value * sign;
}

int _atoi(const char** stringPtr)
{
    int         sign;
    int         value;
    int         c;
    const char* string;

    string = *stringPtr;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
        {
            return 0;
        }
        string++;
    }

    // check sign
    switch (*string)
    {
    case '+':
        string++;
        sign = 1;
        break;
    case '-':
        string++;
        sign = -1;
        break;
    default:
        sign = 1;
        break;
    }

    // read digits
    value = 0;
    do
    {
        c = *string++;
        if (c < '0' || c > '9')
        {
            break;
        }
        c -= '0';
        value = value * 10 + c;
    } while (1);

    // not handling 10e10 notation...

    *stringPtr = string;

    return value * sign;
}

int abs(int n)
{
    return n < 0 ? -n : n;
}

double fabs(double x)
{
    return x < 0 ? -x : x;
}

#define ALT 0x00000001       /* alternate form */
#define HEXPREFIX 0x00000002 /* add 0x or 0X prefix */
#define LADJUST 0x00000004   /* left adjustment */
#define LONGDBL 0x00000008   /* long double */
#define LONGINT 0x00000010   /* long integer */
#define QUADINT 0x00000020   /* quad integer */
#define SHORTINT 0x00000040  /* short integer */
#define ZEROPAD 0x00000080   /* zero (as opposed to blank) pad */
#define FPT 0x00000100       /* floating point number */

#define to_digit(c) ((c) - '0')
#define is_digit(c) ((unsigned)to_digit(c) <= 9)
#define to_char(n) ((n) + '0')

void AddInt(char** buf_p, int val, int width, int flags)
{
    char  text[32];
    int   digits;
    int   signedVal;
    char* buf;

    digits    = 0;
    signedVal = val;
    if (val < 0)
    {
        val = -val;
    }
    do
    {
        text[digits++] = '0' + val % 10;
        val /= 10;
    } while (val);

    if (signedVal < 0)
    {
        text[digits++] = '-';
    }

    buf = *buf_p;

    if (!(flags & LADJUST))
    {
        while (digits < width)
        {
            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
            width--;
        }
    }

    while (digits--)
    {
        *buf++ = text[digits];
        width--;
    }

    if (flags & LADJUST)
    {
        while (width--)
        {
            *buf++ = (flags & ZEROPAD) ? '0' : ' ';
        }
    }

    *buf_p = buf;
}

void AddFloat(char** buf_p, float fval, int width, int prec)
{
    char  text[32];
    int   digits;
    float signedVal;
    char* buf;
    int   val;

    // get the sign
    signedVal = fval;
    if (fval < 0)
    {
        fval = -fval;
    }

    // write the float number
    digits = 0;
    val    = (int)fval;
    do
    {
        text[digits++] = '0' + val % 10;
        val /= 10;
    } while (val);

    if (signedVal < 0)
    {
        text[digits++] = '-';
    }

    buf = *buf_p;

    while (digits < width)
    {
        *buf++ = ' ';
        width--;
    }

    while (digits--)
    {
        *buf++ = text[digits];
    }

    *buf_p = buf;

    if (prec < 0)
        prec = 6;
    // write the fraction
    digits = 0;
    while (digits < prec)
    {
        fval -= (int)fval;
        fval *= 10.0;
        val            = (int)fval;
        text[digits++] = '0' + val % 10;
    }

    if (digits > 0)
    {
        buf    = *buf_p;
        *buf++ = '.';
        for (prec = 0; prec < digits; prec++)
        {
            *buf++ = text[prec];
        }
        *buf_p = buf;
    }
}

void AddString(char** buf_p, char* string, int width, int prec)
{
    int   size;
    char* buf;

    buf = *buf_p;

    if (string == NULL)
    {
        string = "(null)";
        prec   = -1;
    }

    if (prec >= 0)
    {
        for (size = 0; size < prec; size++)
        {
            if (string[size] == '\0')
            {
                break;
            }
        }
    }
    else
    {
        size = strlen(string);
    }

    width -= size;

    while (size--)
    {
        *buf++ = *string++;
    }

    while (width-- > 0)
    {
        *buf++ = ' ';
    }

    *buf_p = buf;
}

int vsprintf(char* buffer, const char* fmt, va_list argptr)
{
    int*  arg;
    char* buf_p;
    char  ch;
    int   flags;
    int   width;
    int   prec;
    int   n;
    char  sign;

    buf_p = buffer;
    arg   = (int*)argptr;

    while (1)
    {
        // run through the format string until we hit a '%' or '\0'
        for (ch = *fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
        {
            *buf_p++ = ch;
        }
        if (ch == '\0')
        {
            goto done;
        }

        // skip over the '%'
        fmt++;

        // reset formatting state
        flags = 0;
        width = 0;
        prec  = -1;
        sign  = '\0';

    rflag:
        ch = *fmt++;
    reswitch:
        switch (ch)
        {
        case '-':
            flags |= LADJUST;
            goto rflag;
        case '.':
            n = 0;
            while (is_digit((ch = *fmt++)))
            {
                n = 10 * n + (ch - '0');
            }
            prec = n < 0 ? -1 : n;
            goto reswitch;
        case '0':
            flags |= ZEROPAD;
            goto rflag;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            n = 0;
            do
            {
                n  = 10 * n + (ch - '0');
                ch = *fmt++;
            } while (is_digit(ch));
            width = n;
            goto reswitch;
        case 'c':
            *buf_p++ = (char)*arg;
            arg++;
            break;
        case 'd':
        case 'i':
            AddInt(&buf_p, *arg, width, flags);
            arg++;
            break;
        case 'f':
            AddFloat(&buf_p, *(double*)arg, width, prec);
#ifdef __LCC__
            arg += 1; // everything is 32 bit in my compiler
#else
            arg += 2;
#endif
            break;
        case 's':
            AddString(&buf_p, (char*)*arg, width, prec);
            arg++;
            break;
        case '%':
            *buf_p++ = ch;
            break;
        default:
            *buf_p++ = (char)*arg;
            arg++;
            break;
        }
    }

done:
    *buf_p = 0;
    return buf_p - buffer;
}

void VM_Com_Printf(const char *string);
void VM_Com_Error(int level, const char *string);

void GDR_DECL G_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[1024];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    VM_Com_Printf(msg);
}

void GDR_DECL Com_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[1024];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    VM_Com_Printf(msg);
}

void GDR_DECL Com_Error(int level, const char *fmt, ...)
{
    va_list argptr;
    char msg[1024];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    VM_Com_Error(level, msg);
}
