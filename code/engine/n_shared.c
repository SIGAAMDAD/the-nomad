#if defined(GDR_DLLCOMPILE) || defined(__cplusplus)
#include "../engine/n_shared.h"
#else
#include "n_shared.h"
#endif
#include "../rendergl/rgl_public.h"

#ifdef __cplusplus
extern renderImport_t ri;
#endif

const byte locase[ 256 ] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
	0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x5b,0x5c,0x5d,0x5e,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
	0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
	0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,
	0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,
	0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
	0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,
	0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,
	0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,
	0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

uint32_t crc32_buffer(const byte *buf, uint32_t len)
{
	static uint32_t crc32_table[256];
	static qboolean crc32_inited = qfalse;

	uint32_t crc = UINT_MAX;

	if (!crc32_inited) {
		uint32_t c;
		int i, j;

		for (i = 0; i < 256; i++) {
			c = i;
			for (j = 0; j < 8; j++)
				c = (c & 1) ? (c >> 1) ^ 0xEDB88320UL : c >> 1;
			
			crc32_table[i] = c;
		}
		crc32_inited = qtrue;
	}

	while (len--)
		crc = crc32_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

	return crc ^ UINT_MAX;
}


#ifndef __cplusplus
qboolean N_strtobool(const char* s)
{
	return N_stricmp(s, "true") ? qtrue : qfalse;
}
const char* N_booltostr(qboolean b)
{
	return b ? "true" : "false";
}
#endif

qboolean Key_IsPressed(qboolean **keys, uint32_t code)
{
	return (*keys)[code];
}

/*
Com_GenerateHashValue: used in renderer and filesystem
*/
// ASCII lowcase conversion table with '\\' turned to '/' and '.' to '\0'
static const byte hash_locase[ 256 ] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
	0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x00,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x5b,0x2f,0x5d,0x5e,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
	0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
	0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,
	0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,
	0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
	0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,
	0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,
	0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,
	0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

uint64_t Com_GenerateHashValue( const char *fname, const uint64_t size )
{
	const byte *s;
	uint64_t hash;
	int c;

	s = (byte *)fname;
	hash = 0;
	
	while ( (c = hash_locase[(byte)*s++]) != '\0' ) {
		hash = hash * 101 + c;
	}
	
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (size-1);

	return hash;
}

const char *Com_StringContains( const char *str1, const char *str2, uint64_t len2 )
{
	int64_t len, i, j;

	len = strlen(str1) - len2;
	for (i = 0; i <= len; i++, str1++) {
		for (j = 0; str2[j]; j++) {
			if (locase[(byte)str1[j]] != locase[(byte)str2[j]]) {
				break;
			}
		}
		if (!str2[j]) {
			return str1;
		}
	}
	return NULL;
}

qboolean Com_FilterExt( const char *filter, const char *name )
{
	char buf[ MAX_TOKEN_CHARS ];
	const char *ptr;
	uint32_t i;

	while ( *filter ) {
		if ( *filter == '*' ) {
			filter++;
			for ( i = 0; *filter != '\0' && i < sizeof(buf)-1; i++ ) {
				if ( *filter == '*' || *filter == '?' )
					break;
				buf[i] = *filter++;
			}
			buf[ i ] = '\0';
			if ( i ) {
				ptr = Com_StringContains( name, buf, i );
				if ( !ptr )
					return qfalse;
				name = ptr + i;
			} else if ( *filter == '\0' ) {
				return qtrue;
			}
		}
		else if ( *filter == '?' ) {
			if ( *name == '\0' )
				return qfalse;
			filter++;
			name++;
		}
		else {
			if ( locase[(byte)*filter] != locase[(byte)*name] )
				return qfalse;
			filter++;
			name++;
		}
	}
	if ( *name ) {
		return qfalse;
	}
	return qtrue;
}


int Com_Filter( const char *filter, const char *name )
{
	char buf[ MAX_TOKEN_CHARS ];
	const char *ptr;
	int i, found;

	while (*filter) {
		if (*filter == '*') {
			filter++;
			for (i = 0; *filter; i++) {
				if (*filter == '*' || *filter == '?')
					break;
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if ( i ) {
				ptr = Com_StringContains( name, buf, i );
				if ( !ptr )
					return qfalse;
				name = ptr + i;
			}
		}
		else if (*filter == '?') {
			filter++;
			name++;
		}
		else if (*filter == '[' && *(filter+1) == '[') {
			filter++;
		}
		else if (*filter == '[') {
			filter++;
			found = qfalse;
			while(*filter && !found) {
				if (*filter == ']' && *(filter+1) != ']') break;
				if (*(filter+1) == '-' && *(filter+2) && (*(filter+2) != ']' || *(filter+3) == ']')) {
					if (locase[(byte)*name] >= locase[(byte)*filter] &&
						locase[(byte)*name] <= locase[(byte)*(filter+2)])
							found = qtrue;
					filter += 3;
				}
				else {
					if (locase[(byte)*filter] == locase[(byte)*name])
						found = qtrue;
					filter++;
				}
			}
			if (!found) return qfalse;
			while (*filter) {
				if (*filter == ']' && *(filter+1) != ']') break;
				filter++;
			}
			filter++;
			name++;
		}
		else {
			if (locase[(byte)*filter] != locase[(byte)*name])
				return qfalse;
			filter++;
			name++;
		}
	}
	return qtrue;
}

int Com_FilterPath( const char *filter, const char *name )
{
	int i;
	char new_filter[MAX_GDR_PATH];
	char new_name[MAX_GDR_PATH];

	for (i = 0; i < MAX_GDR_PATH-1 && filter[i]; i++) {
		if ( filter[i] == '\\' || filter[i] == ':' ) {
			new_filter[i] = '/';
		}
		else {
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';
	for (i = 0; i < MAX_GDR_PATH-1 && name[i]; i++) {
		if ( name[i] == '\\' || name[i] == ':' ) {
			new_name[i] = '/';
		}
		else {
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';
	return Com_Filter( new_filter, new_name );
}

qboolean Com_HasPatterns( const char *str )
{
	int c;

	while ( (c = *str++) != '\0' ) {
		if ( c == '*' || c == '?' ) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
==================
COM_DefaultExtension

if path doesn't have an extension, then append
 the specified one (which should include the .)
==================
*/
void COM_DefaultExtension( char *path, uint64_t maxSize, const char *extension )
{
	const char *dot = strrchr(path, '.'), *slash;
	if (dot && ((slash = strrchr(path, '/')) == NULL || slash < dot))
		return;
	else
		N_strcat(path, maxSize, extension);
}


void COM_StripExtension(const char *in, char *out, uint64_t destsize)
{
	const char *dot = (char *)strrchr(in, '.'), *slash;

	if (dot && ((slash = (char *)strrchr(in, '/')) == NULL || slash < dot))
		destsize = (destsize < dot-in+1 ? destsize : dot-in+1);

	if ( in == out && destsize > 1 )
		out[destsize-1] = '\0';
	else
		N_strncpy(out, in, destsize);
}

void CopyShortSwap(void *dest, void *src)
{
	byte *to = (byte *)dest, *from = (byte *)src;

	to[0] = from[1];
	to[1] = from[0];
}

void CopyIntSwap(void *dest, void *src)
{
	byte *to = (byte *)dest, *from = (byte *)src;

	to[0] = from[3];
	to[1] = from[2];
	to[2] = from[1];
	to[3] = from[0];
}

void CopyLongSwap(void *dest, void *src)
{
	byte *to = (byte *)dest, *from = (byte *)src;

	to[0] = from[7];
	to[1] = from[6];
	to[2] = from[5];
	to[3] = from[4];
	to[4] = from[3];
	to[5] = from[2];
	to[6] = from[1];
	to[7] = from[0];
}

/*
=====================================================================

Library Replacement Functions

=====================================================================
*/

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
			((long *)dest)[count] = ((long *)src)[count];
			count -= 4;
		}
	}
	else if (( ( (long)dest | (long)src | count) & 3) == 0 ) {
		count>>=2;
		for (i = 0; i < count; i++)
			((int *)dest)[i] = ((int *)src)[i];
	}
	else
		for (i = 0; i < count; i++)
			((char *)dest)[i] = ((char *)src)[i];
}

int N_memcmp (const void *ptr1, const void *ptr2, size_t count)
{
	while (count--) {
		if (((char *)ptr1)[count] != ((char *)ptr2)[count])
			return -1;
	}
	return 1;
}


#ifdef _WIN32
/*
=============
N_vsnprintf
 
Special wrapper function for Microsoft's broken _vsnprintf() function. mingw-w64
however, uses Microsoft's broken _vsnprintf() function.
=============
*/
int N_vsnprintf( char *str, size_t size, const char *format, va_list ap )
{
	int retval;
	
	retval = _vsnprintf( str, size, format, ap );

	if ( retval < 0 || (size_t)retval == size ) {
		// Microsoft doesn't adhere to the C99 standard of vsnprintf,
		// which states that the return value must be the number of
		// bytes written if the output string had sufficient length.
		//
		// Obviously we cannot determine that value from Microsoft's
		// implementation, so we have no choice but to return size.
		
		str[size - 1] = '\0';
		return size;
	}
	
	return retval;
}
#endif

int N_isprint( int c )
{
	if ( c >= 0x20 && c <= 0x7E )
		return ( 1 );
	return ( 0 );
}


int N_islower( int c )
{
	if (c >= 'a' && c <= 'z')
		return ( 1 );
	return ( 0 );
}


int N_isupper( int c )
{
	if (c >= 'A' && c <= 'Z')
		return ( 1 );
	return ( 0 );
}


int N_isalpha( int c )
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return ( 1 );
	return ( 0 );
}

qboolean N_isintegral(float f)
{
	return (qboolean)((int)f == f);
}


qboolean N_isanumber( const char *s )
{
#ifdef Q3_VM
    //FIXME: implement
    return qfalse;
#else
    char *p;

	if( *s == '\0' )
        return qfalse;

	strtod( s, &p );

    return (qboolean)(*p == '\0');
#endif
}

#ifndef Q3_VM
void N_itoa(char *buf, uint64_t bufsize, int i)
{
	snprintf(buf, bufsize, "%i", i);
}

void N_ftoa(char *buf, uint64_t bufsize, float f)
{
	snprintf(buf, bufsize, "%f", f);
}
#endif

void N_strcpy (char *dest, const char *src)
{
	char *d = dest;
	const char *s = src;
	while (*s)
		*d++ = *s++;
	
	*d++ = 0;
}

void Com_TruncateLongString( char *buffer, const char *s )
{
	uint64_t length = strlen( s );

	if( length <= TRUNCATE_LENGTH )
		N_strncpyz( buffer, s, TRUNCATE_LENGTH );
	else {
		N_strncpyz( buffer, s, ( TRUNCATE_LENGTH / 2 ) - 3 );
		N_strcat( buffer, TRUNCATE_LENGTH, " ... " );
		N_strcat( buffer, TRUNCATE_LENGTH, s + length - ( TRUNCATE_LENGTH / 2 ) + 3 );
	}
}


void N_strncpyz (char *dest, const char *src, size_t count)
{
#if defined(Q3_VM) || !defined(__cplusplus)
	if (!dest)
		SG_Error( "N_strncpyz: NULL dest");
	if (!src)
		SG_Error( "N_strncpyz: NULL src");
	if (count < 1)
		SG_Error( "N_strncpyz: bad count");
#elif defined(GDR_DLLCOMPILE)
	if (!dest)
		ri.N_Error("N_strncpyz: NULL dest");
	if (!src)
		ri.N_Error("N_strncpyz: NULL src");
	if (count < 1)
		ri.N_Error("N_strncpyz: bad count");
#else
	if (!dest)
		N_Error("N_strncpyz: NULL dest");
	if (!src)
		N_Error("N_strncpyz: NULL src");
	if (count < 1)
		N_Error("N_strncpyz: bad count");
#endif
	
#if 1 
	// do not fill whole remaining buffer with zeros
	// this is obvious behavior change but actually it may affect only buggy QVMs
	// which passes overlapping or short buffers to cvar reading routines
	// what is rather good than bad because it will no longer cause overwrites, maybe
	while ( --count > 0 && (*dest++ = *src++) != '\0' );
	*dest = '\0';
#else
	strncpy( dest, src, count-1 );
	dest[ count-1 ] = '\0';
#endif
}

void N_strncpy (char *dest, const char *src, size_t count)
{
	while (*src && count--)
		*dest++ = *src++;

	if (count)
		*dest++ = 0;
}

/*
==================
Com_CharIsOneOfCharset
==================
*/
static qboolean Com_CharIsOneOfCharset( char c, const char *set )
{
	uint64_t i, n = (uint64_t)(strlen(set));

	for( i = 0; i < n; i++ ) {
		if( set[ i ] == c )
			return qtrue;
	}

	return qfalse;
}

/*
==================
Com_SkipCharset
==================
*/
const char *Com_SkipCharset( const char *s, const char *sep )
{
	const char	*p = s;

	while( p ) {
		if( Com_CharIsOneOfCharset( *p, sep ) )
			p++;
		else
			break;
	}

	return p;
}


/*
Not thread safe
*/
const char* GDR_DECL va(const char *format, ...)
{
	char *buf;
	va_list argptr;
	static uint32_t index = 0;
	static char string[2][32000];	// in case va is called by nested functions

	buf = string[ index ];
	index ^= 1;

	va_start( argptr, format );
#ifdef Q3_VM
	vsprintf( buf, format, argptr );
#else
	N_vsnprintf( buf, sizeof(*string), format, argptr );
#endif
	va_end( argptr );

	return buf;
}

/*
==================
Com_SkipTokens
==================
*/
const char *Com_SkipTokens( const char *s, uint64_t numTokens, const char *sep )
{
	uint64_t sepCount = 0;
	const char	*p = s;

	while( sepCount < numTokens ) {
		if( Com_CharIsOneOfCharset( *p++, sep ) ) {
			sepCount++;
			while( Com_CharIsOneOfCharset( *p, sep ) )
				p++;
		}
		else if( *p == '\0' )
			break;
	}

	if( sepCount == numTokens )
		return p;
	else
		return s;
}


char *N_strlwr(char *s1)
{
	char	*s;

	s = s1;
	while ( *s ) {
		*s = locase[(byte)*s];
		s++;
	}
	return s1;
}

char *N_strupr(char *s1)
{
	char *s;

	s = s1;
	while (*s) {
		if (*s >= 'a' && *s <= 'z')
			*s = *s - 'a' + 'A';
		s++;
	}
	return s1;
}

// never goes past bounds or leaves without a terminating 0
void N_strcat(char *dest, size_t size, const char *src)
{
	size_t l1;

	l1 = strlen(dest);
	if (l1 >= size)
#if defined(Q3_VM) || !defined(__cplusplus)
		SG_Error( "N_strcat: already overflowed" );
#elif defined(GDR_DLLCOMPILE)
		ri.N_Error("N_strcat: already overflowed");
#else
		N_Error("N_strcat: already overflowed");
#endif
	
	N_strncpy( dest + l1, src, size - l1 );
}

char *N_stradd(char *dst, const char *src)
{
	char c;
	while ( (c = *src++) != '\0' )
		*dst++ = c;
	*dst = '\0';
	return dst;
}


/*
* Find the first occurrence of find in s.
*/
const char *N_stristr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		if (c >= 'a' && c <= 'z') {
	    	c -= ('a' - 'A');
		}
 	   	len = strlen(find);
    	do {
    		do {
        		if ((sc = *s++) == 0)
          			return NULL;
        		if (sc >= 'a' && sc <= 'z') {
          			sc -= ('a' - 'A');
        		}
      		} while (sc != c);
    	} while (N_stricmpn(s, find, len) != 0);
   		s--;
  	}
  	return s;
}

int N_replace(const char *str1, const char *str2, char *src, size_t max_len)
{
	size_t len1, len2, count;
	ssize_t d;
	const char *s0, *s1, *s2, *max;
	char *match, *dst;

	match = strstr(src, str1);

	if (!match)
		return 0;

	count = 0; // replace count

    len1 = strlen(str1);
    len2 = strlen(str2);
    d = len2 - len1;

    if (d > 0) { // expand and replace mode
        max = src + max_len;
        src += strlen(src);

        do { // expand source string
			s1 = src;
            src += d;
            if (src >= max)
                return count;
            dst = src;
            
            s0 = match + len1;

            while (s1 >= s0)
                *dst-- = *s1--;
			
			// replace match
            s2 = str2;
			while (*s2)
                *match++ = *s2++;
			
            match = strstr(match, str1);

            count++;
		} while (match);

        return count;
    } 
    else if (d < 0) { // shrink and replace mode
        do  { // shrink source string
            s1 = match + len1;
            dst = match + len2;
            while ( (*dst++ = *s1++) != '\0' );
			
			//replace match
            s2 = str2;
			while ( *s2 ) {
				*match++ = *s2++;
			}

            match = strstr( match, str1 );

            count++;
        } 
        while ( match );

        return count;
    }
    else {
	    do { // just replace match
    	    s2 = str2;
			while (*s2)
				*match++ = *s2++;

    	    match = strstr(match, str1);
    	    count++;
		}  while (match);
	}

	return count;
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
	
	return 0;
}

qboolean N_streq(const char *str1, const char *str2)
{
	const char *s1 = str1;
	const char *s2 = str2;
	
	while (*s2 && *s1) {
		if (*s1++ != *s2++)
			return qfalse;
	}
	return qtrue;
}

qboolean N_strneq(const char *str1, const char *str2, size_t n)
{
	const char *s1 = str1;
	const char *s2 = str2;

	while (*s1 && n) {
		if (*s1++ != *s2++)
			return qfalse;
		n--;
	}
	return qtrue;
}

int N_strncmp( const char *s1, const char *s2, size_t n )
{
	int c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int N_stricmpn (const char *str1, const char *str2, size_t n)
{
	int c1, c2;

	// bk001129 - moved in 1.17 fix not in id codebase
    if (str1 == NULL) {
    	if (str2 == NULL )
            return 0;
        else
            return -1;
    }
    else if (str2 == NULL)
        return 1;


	
	do {
		c1 = *str1++;
		c2 = *str2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int N_stricmp( const char *s1, const char *s2 ) 
{
	unsigned char c1, c2;

	if (s1 == NULL)  {
		if (s2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (s2 == NULL)
		return 1;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 != c2) {
			if ( c1 <= 'Z' && c1 >= 'A' )
				c1 += ('a' - 'A');

			if ( c2 <= 'Z' && c2 >= 'A' )
				c2 += ('a' - 'A');

			if ( c1 != c2 ) 
				return c1 < c2 ? -1 : 1;
		}
	} while ( c1 != '\0' );

	return 0;
}

int N_atoi (const char *s)
{
	int val;
	int sign;
	int c;
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

/*
================
N_isnan

Don't pass doubles to this
================
*/
int N_isnan( float x )
{
	floatint_t fi;

	fi.f = x;
	fi.u &= 0x7FFFFFFF;
	fi.u = 0x7F800000 - fi.u;

	return (int)( fi.u >> 31 );
}
//------------------------------------------------------------------------


/*
================
N_isfinite
================
*/
static int N_isfinite( float f )
{
	floatint_t fi;
	fi.f = f;

	if ( fi.u == 0xFF800000 || fi.u == 0x7F800000 )
		return 0; // -INF or +INF

	fi.u = 0x7F800000 - (fi.u & 0x7FFFFFFF);
	if ( (int)( fi.u >> 31 ) )
		return 0; // -NAN or +NAN

	return 1;
}


/*
================
N_atof
================
*/
float N_atof( const char *str )
{
	float f;

	f = atof( str );

	// modern C11-like implementations of atof() may return INF or NAN
	// which breaks all FP code where such values getting passed
	// and effectively corrupts range checks for cvars as well
	if ( !N_isfinite( f ) )
		return 0.0f;

	return f;
}


#ifndef Q3_VM
/*
================
N_log2f
================
*/
float N_log2f( float f )
{
	const float v = logf( f );
	return v / M_LN2;
}


/*
================
N_exp2f
================
*/
float N_exp2f( float f )
{
	return powf( 2.0f, f );
}


/*
=====================
N_acos

the msvc acos doesn't always return a value between -PI and PI:

int i;
i = 1065353246;
acos(*(float*) &i) == -1.#IND0

=====================
*/
float N_acos(float c)
{
	float angle;

	angle = acos(c);

	if (angle > M_PI) {
		return (float)M_PI;
	}
	if (angle < -M_PI) {
		return (float)M_PI;
	}
	return angle;
}
#endif

#ifdef Q3_VM
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
#endif
