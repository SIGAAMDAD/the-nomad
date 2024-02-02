#include "sg_local.h"

// 12 KiB of static memory for the vm to use
#define MEMPOOL_SIZE (12*1024*1024)
static char mempool[MEMPOOL_SIZE];
static int allocPoint;

#define STRINGPOOL_SIZE (8*1024)

#define HASH_TABLE_SIZE 2048

typedef struct stringDef_s {
	struct stringDef_s *next;
	const char *str;
} stringDef_t;

static int strPoolIndex;
static char strPool[STRINGPOOL_SIZE];

static int strHandleCount;
static stringDef_t *strHandle[HASH_TABLE_SIZE];

static qboolean outOfMemory;

const char *String_Alloc( const char *p )
{
	int len;
	uint64_t hash;
	stringDef_t *str, *last;
	static const char *staticNULL = "";

	if (p == NULL) {
		return NULL;
	}

	if (*p == 0) {
		return staticNULL;
	}

	hash = Com_GenerateHashValue( p, HASH_TABLE_SIZE );

	str = strHandle[hash];
	while (str) {
		if (strcmp(p, str->str) == 0) {
			return str->str;
		}
		str = str->next;
	}

	len = strlen(p);
	if (len + strPoolIndex + 1 < STRINGPOOL_SIZE) {
		int ph = strPoolIndex;
		strcpy(&strPool[strPoolIndex], p);
		strPoolIndex += len + 1;

		str = strHandle[hash];
		last = str;
		while (str && str->next) {
			last = str;
			str = str->next;
		}

		str = (stringDef_t *)SG_MemAlloc( sizeof(stringDef_t) );
		str->next = NULL;
		str->str = &strPool[ph];
		if (last) {
			last->next = str;
		} else {
			strHandle[hash] = str;
		}
		return &strPool[ph];
	}
	return NULL;
}

void String_Report( void )
{
	float f;
	Con_Printf( "Memory/String Pool Info\n");
	Con_Printf( "----------------\n");
	
    f = strPoolIndex;
	f /= STRINGPOOL_SIZE;
	f *= 100;
	Con_Printf( "String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex, STRINGPOOL_SIZE );
	
    f = allocPoint;
	f /= MEMPOOL_SIZE;
	f *= 100;
	Con_Printf( "Memory Pool is %.1f%% full, %i bytes out of %i used.\n", f, allocPoint, MEMPOOL_SIZE );
}

void *SG_MemAlloc( int size )
{
    char *buf;

    if ( !size ) {
        trap_Error( "SG_MemAlloc: bad size" );
    }

    size = PAD( size, (unsigned)16 ); // round to 16-byte alignment

    if ( allocPoint + size >= sizeof(mempool) ) {
        trap_Print( COLOR_RED "(ERROR) SG_MemAlloc: not enough vm memory\n" );
		outOfMemory = qtrue;
        return NULL;
    }

    buf = &mempool[ allocPoint ];
    allocPoint += size;

    // zero init
    memset( buf, 0, size );

    return buf;
}

int SG_MemoryRemaining( void ) {
    return sizeof(mempool) - allocPoint;
}

qboolean SG_OutOfMemory( void ) {
	return outOfMemory;
}

void SG_ClearToMemoryMark( int mark ) {
    if ( mark < 0 || mark > allocPoint ) {
        trap_Error( "SG_ClearToMemoryMark: invalid memory mark" );
    }
    allocPoint = mark;
}

int SG_MakeMemoryMark( void ) {
    return allocPoint;
}

void SG_MemInit( void )
{
    int i;

	outOfMemory = qfalse;

    memset( mempool, 0, sizeof(mempool) );
    memset( strPool, 0, sizeof(strPool) );

	for ( i = 0; i < HASH_TABLE_SIZE; i++ ) {
		strHandle[i] = 0;
	}
	strHandleCount = 0;
	strPoolIndex = 0;
}
