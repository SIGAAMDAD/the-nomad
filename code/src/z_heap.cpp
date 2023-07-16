#include "n_shared.h"

#define DEFAULT_HEAP_SIZE (1800*1024*1024)
#define MIN_HEAP_SIZE (1000*1024*1024)
#define DEFAULT_ZONE_SIZE (400*1024*1024)

file_t logfile;

byte *hunkbase = NULL;
uint64_t hunksize = 0;

#if 0
typedef struct {
	uint64_t magic;
	uint64_t size;
} hunkHeader_t;

typedef struct {
	uint64_t mark;
	uint64_t permanent;
	uint64_t temp;
	uint64_t tempHighwater;
} hunkUsed_t;

typedef struct hunkblock_s {
	uint64_t size;
	byte printed;
	struct hunkblock_s *next;
	const char *label;
	const char *file;
	uint64_t line;
} hunkblock_t;

hunkUsed_t hunk_low, hunk_high;
hunkUsed_t *hunk_permanent, *hunk_temp;
#endif

void Memory_Shutdown(void)
{
	Con_Printf("Memory_Shutdown: deallocating allocation daemons");
	free(hunkbase);
    Mem_Shutdown();
}

static void Com_Meminfo_f(void)
{
	uint64_t unused;

#if 0
	Con_Printf( "%8li bytes total hunk", hunksize );
	Con_Printf( " " );
	Con_Printf( "%8li low mark\n", hunk_low.mark );
	Con_Printf( "%8li low permanent", hunk_low.permanent );
	if ( hunk_low.temp != hunk_low.permanent ) {
		Con_Printf( "%8li low temp", hunk_low.temp );
	}
	Con_Printf( "%8li low tempHighwater", hunk_low.tempHighwater );
	Con_Printf( " " );
	Con_Printf( "%8li high mark", hunk_high.mark );
	Con_Printf( "%8li high permanent", hunk_high.permanent );
	if ( hunk_high.temp != hunk_high.permanent ) {
		Con_Printf( "%8li high temp", hunk_high.temp );
	}
	Con_Printf( "%8li high tempHighwater", hunk_high.tempHighwater );
	Con_Printf( " " );
	Con_Printf( "%8li total hunk in use", hunk_low.permanent + hunk_high.permanent );
	unused = 0;
	if ( hunk_low.tempHighwater > hunk_low.permanent ) {
		unused += hunk_low.tempHighwater - hunk_low.permanent;
	}
	if ( hunk_high.tempHighwater > hunk_high.permanent ) {
		unused += hunk_high.tempHighwater - hunk_high.permanent;
	}
	Con_Printf( "%8li unused highwater", unused );
	Con_Printf( " " );
#endif

//	Zone_Stats( "main", mainzone, !Q_stricmp( Cmd_Argv(1), "main" ) || !Q_stricmp( Cmd_Argv(1), "all" ), &st );
//	Con_Printf( "%8li bytes total main zone\n\n", mainzone->size );
//	Con_Printf( "%8li bytes in %i main zone blocks%s\n", st.zoneBytes, st.zoneBlocks,
//		st.zoneSegments > 1 ? va( " and %i segments", st.zoneSegments ) : "" );
//	Con_Printf( "        %8li bytes in botlib\n", st.botlibBytes );
//	Con_Printf( "        %8li bytes in renderer\n", st.rendererBytes );
//	Con_Printf( "        %8li bytes in other\n", st.zoneBytes - ( st.botlibBytes + st.rendererBytes ) );
//	Con_Printf( "        %8li bytes in %i free blocks\n", st.freeBytes, st.freeBlocks );
//	if ( st.freeBlocks > 1 ) {
//		Con_Printf( "        (largest: %i bytes, smallest: %i bytes)\n\n", st.freeLargest, st.freeSmallest );
//	}
//
//	Zone_Stats( "small", smallzone, !Q_stricmp( Cmd_Argv(1), "small" ) || !Q_stricmp( Cmd_Argv(1), "all" ), &st );
//	Con_Printf( "%8li bytes total small zone\n\n", smallzone->size );
//	Con_Printf( "%8li bytes in %i small zone blocks%s\n", st.zoneBytes, st.zoneBlocks,
//		st.zoneSegments > 1 ? va( " and %i segments", st.zoneSegments ) : "" );
//	Con_Printf( "        %8li bytes in %i free blocks\n", st.freeBytes, st.freeBlocks );
//	if ( st.freeBlocks > 1 ) {
//		Con_Printf( "        (largest: %i bytes, smallest: %i bytes)\n\n", st.freeLargest, st.freeSmallest );
//	}
}

static uint64_t Com_TouchMemory(void)
{
	uint64_t i, j, sum;

	sum = 0;
#if 0
	j = hunk_low.permanent >> 2;
	for ( i = 0 ; i < j ; i+=64 ) {			// only need to touch each page
		sum += ((uint32_t *)hunkbase)[i];
	}

	i = ( hunksize - hunk_high.permanent ) >> 2;
	j = hunk_high.permanent >> 2;
	for (  ; i < j ; i+=64 ) {			// only need to touch each page
		sum += ((uint32_t *)hunkbase)[i];
	}
#endif

	Z_TouchMemory(&sum);

	return sum;
}

void Memory_Init(void)
{
	Con_Printf("Memory_Init: initializing allocation daemons");

	// make sure the file system has allocated and "not" freed any temp blocks
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redundant routines in the file system utilizing different
	// memory systems
#if 0
    if ( FS_LoadStack() != 0 ) {
		N_Error( "Hunk initialization failed. File system load stack not zero" );
	}
#endif

	uint64_t zonesize;

    int i = I_GetParm("-ram");
    if (i != -1) {
        if (i+1 >= myargc)
            N_Error("Memory_Init: you must specify the amount of ram in MB after -ram");
        else
            hunksize = N_atoi(myargv[i+1]) * 1024 * 1024;
    }
    else {
        hunksize = DEFAULT_HEAP_SIZE;
    }
	
    hunkbase = (byte *)calloc(hunksize, sizeof(byte));
    if (!hunkbase) {
        N_Error("Memory_Init: malloc() failed on %lu bytes when allocating the hunk", hunksize);
	}
	// initialize it all
	Hunk_Clear();

	Con_Printf("Initialized heap from %p -> %p (%lu MiB)", (void *)hunkbase, (void *)(hunkbase + hunksize), hunksize >> 20);
	
	// initialize the zone heap
	Z_Init();

	Cmd_AddCommand("meminfo", Com_Meminfo_f);
#if 0
	Cmd_AddCommand("hunklogsmall", Hunk_SmallLog);
	Cmd_AddCommand("hunklog", Hunk_Log);
#endif
}


void Mem_Info(void)
{
	Com_Meminfo_f();
    Z_Print(true);
}