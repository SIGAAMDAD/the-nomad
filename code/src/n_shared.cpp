#include "n_shared.h"

void* N_memset (void *dest, int fill, size_t count)
{
	size_t i;
	
	if ( (((long)dest | count) & 3) == 0) {
		count >>= 2;
		fill = fill | (fill<<8) | (fill<<16) | (fill<<24);
		for (i = 0; i < count; i++)
			((int *)dest)[i] = fill;
	}
	else
		for (i = 0; i < count; i++)
			((char *)dest)[i] = fill;
    
    return dest;
}

void* N_memchr (void *ptr, int c, size_t count)
{
	while (--count) {
		if (((char *)ptr)[count] == c)
			return (void *)&((char *)ptr)[count];
	}
	return NULL;
}

void N_memcpy (void *dest, const void *src, size_t count)
{
	size_t i;
	if (( ( (long)dest | (long)src | count) & 7) == 0) {
		while (count >= 4) {
			((long *)dest)[count] = ((const long *)src)[count];
			count -= 4;
		}
	}
	else if (( ( (long)dest | (long)src | count) & 3) == 0 ) {
		count>>=2;
		for (i = 0; i < count; i++)
			((int *)dest)[i] = ((const int *)src)[i];
	}
	else
		for (i = 0; i < count; i++)
			((char *)dest)[i] = ((const char *)src)[i];
}

int N_memcmp (const void *ptr1, const void *ptr2, size_t count)
{
	while (count--) {
		if (((const char *)ptr1)[count] != ((const char *)ptr2)[count])
			return -1;
	}
	return 1;
}

void N_strcpy (char *dest, const char *src)
{
	char *d = dest;
	const char *s = src;
	while (*s)
		*d++ = *s++;
	
	*d++ = 0;
}

void N_strncpy (char *dest, const char *src, size_t count)
{
	char *d = dest;
	const char *s = src;
	while (*s && count--)
		*d++ = *s++;

	if (count)
		*d++ = 0;
}

size_t N_strlen (const char *str)
{
	size_t count = 0;
    while (str[count]) {
        ++count;
    }
	return count;
}

char *N_strrchr(char *str, char c)
{
    char *s = str;
    size_t len = N_strlen(s);
    s += len;
    while (len--)
    	if (*--s == c) return s;
    return 0;
}

void N_strcat (char *dest, const char *src)
{
    char *d = dest;
	d += N_strlen(dest);
	N_strcpy (d, src);
}

int N_strcmp (const char *str1, const char *str2)
{
    const char *s1 = str1;
    const char *s2 = str2;
	while (1) {
		if (*s1 != *s2)
			return -1;              // strings not equal    
		if (!*s1)
			return 1;               // strings are equal
		s1++;
		s2++;
	}
	
	return -1;
}

int N_strncmp (const char *str1, const char *str2, size_t count)
{
    const char* s1 = str1;
    const char* s2 = str2;
	while (1) {
		if (!count--)
			return 1;
		if (*s1 != *s2)
			return -1;              // strings not equal    
		if (!*s1)
			return 1;               // strings are equal
		s1++;
		s2++;
	}
	
	return -1;
}


int N_strncasecmp (const char *str1, const char *str2, size_t n)
{
	if (!str1) {
		if (!str2)
			return -1;
		else
			return -1;
	}
	else if (!str2)
		return 1;
	
	int c1, c2;

    const char* s1 = str1;
    const char* s2 = str2;
	
	while (1) {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 1;               // strings are equal until end point
		
		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;              // strings not equal
		}
		if (!c1)
			return 1;               // strings are equal
	}
	
	return -1;
}

int N_strcasecmp (const char *s1, const char *s2)  
{
	return N_strncasecmp (s1, s2, 99999);
}

int GDR_DECL Com_snprintf(char *dest, uint32_t size, const char *fmt, ...)
{
	int len;
	va_list argptr;
	
	va_start(argptr, fmt);
	len = vsnprintf(dest, size, fmt, argptr);
	va_end(argptr);

	if (len >= size) {
		N_Error("Com_snprintf: buffer overflow of %i bytes", len);
	}

	return len;
}

const char* GDR_DECL va(const char *fmt, ...)
{
	char *buf;
	va_list argptr;
	static uint32_t index = 0;
	static char str[2][2048]; // in case its called by nested functions

	buf = str[index];
	index ^= 1;

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	return buf;
}

char* N_stradd(char *dst, const char *src)
{
	const char *s = src;
	char *d = dst;
	char c;
	while ( (c = *s++) != '\0' )
		*d++ = c;
	*d = '\0';
	return dst;
}

qboolean N_streq(const char *str1, const char *str2)
{
	const char *s1 = str1;
	const char *s2 = str2;

	do {
		if (*s1++ != *s2++)
			return qfalse;
	} while (*s1 != '\0');
	return qtrue;
}

qboolean N_strneq(const char *str1, const char *str2, size_t n)
{
	const char *s1 = str1;
	const char *s2 = str2;

	do {
		if (*s1++ != *s2++)
			return qfalse;
	} while (*s1 != '\0' && n--);
	return qtrue;
}

int N_atoi (const char *s)
{
	int             val;
	int             sign;
	int             c;
    const char* str = s;
	
	if (*str == '-') {
		sign = -1;
		str++;
	}
	else
		sign = 1;
		
	val = 0;

    //
    // check for hex
    //
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') ) {
		str += 2;
		while (1) {
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val<<4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val<<4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val<<4) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
    //
    // check for character
    //
	if (str[0] == '\'')
		return sign * str[1];
	
    //
    // assume decimal
    //
	while (1) {
		c = *str++;
		if (c <'0' || c > '9')
			return val*sign;
		val = val*10 + c - '0';
	}
	
	return 0;
}


float N_atof(const char *s)
{
	double			val;
	int             sign;
	int             c;
	int             decimal, total;
    const char *str = s;
	
	if (*str == '-') {
		sign = -1;
		str++;
	}
	else
		sign = 1;
		
	val = 0;

    //
    // check for hex
    //
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') ) {
		str += 2;
		while (1) {
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val*16) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val*16) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val*16) + c - 'A' + 10;
			else
				return val*sign;
		}
	}
	
    //
    // check for character
    //
	if (str[0] == '\'')
		return sign * str[1];
	
    //
    // assume decimal
    //
	decimal = -1;
	total = 0;
	while (1) {
		c = *str++;
		if (c == '.') {
			decimal = total;
			continue;
		}
		if (c <'0' || c > '9')
			break;
		val = val*10 + c - '0';
		total++;
	}

	if (decimal == -1)
		return val*sign;
	while (total > decimal) {
		val /= 10;
		total--;
	}
	
	return val*sign;
}

// returns simply true if strings are exactly equal or false if they are not
bool N_strnbcmp(const char* str1, const char* str2, size_t n)
{
    const char *cmp1 = str1;
    const char *cmp2 = str2;
    while (--n) {
        if (*cmp1++ != *cmp2++)
            return false;
    }
    return true;
}
bool N_strbcmp(const char* str1, const char* str2)
{
    const char *cmp1 = str1;
    const char *cmp2 = str2;
    while (*cmp1 && *cmp2) {
        if (*cmp1++ != *cmp2++)
            return false;
    }
    return true;
}

size_t N_ReadFile(const char *filepath, void *buffer)
{
	if (!buffer) {
		N_Error("N_ReadFile: null buffer");
	}
	FILE* fp = fopen(filepath, "rb");
	if (!fp) {
		N_Error("N_LoadFile: failed to open file %s", filepath);
	}
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	void *buf = Z_Malloc(fsize, TAG_STATIC, &buf, "filebuf");
	if (fread(buf, fsize, 1, fp) == 0) {
		N_Error("N_LoadFile: failed to read %lu bytes from file %s", fsize, filepath);
	}
	N_memcpy(buffer, buf, fsize);
	xfree(buf);
	fclose(fp);
	return fsize;
}
size_t N_FileSize(const char *filepath)
{
	FILE* fp = fopen(filepath, "rb");
	if (!fp) {
		N_Error("N_FileSize: failed to oepn file %s", filepath);
	}
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	fclose(fp);
	return fsize;
}
size_t N_LoadFile(const char *filepath, void **buffer)
{
	FILE* fp = fopen(filepath, "rb");
	if (!fp) {
		N_Error("N_LoadFile: failed to open file %s", filepath);
	}
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	void *buf = Z_Malloc(fsize, TAG_STATIC, &buf, "filebuf");
	if (fread(buf, fsize, 1, fp) == 0) {
		N_Error("N_LoadFile: failed to read %lu bytes from file %s", fsize, filepath);
	}
	*buffer = buf;
	fclose(fp);
	return fsize;
}
void N_WriteFile(const char *filepath, const void *data, size_t size)
{
	FILE* fp = fopen(filepath, "wb");
	if (!fp) {
		Con_Error("N_WriteFile: failed to open file %s", filepath);
		return;
	}
	if (fwrite(data, size, 1, fp) == 0) {
		Con_Error("N_WriteFile: failed to write %lu bytes to file %s", size, filepath);
		return;
	}
	fclose(fp);
}