#include "n_shared.h"

#define DEFAULT_HEAP_SIZE (1800*1024*1024)
#define MIN_HEAP_SIZE (1000*1024*1024)
#define DEFAULT_ZONE_SIZE (400*1024*1024)

file_t logfile;

byte *hunkbase = NULL;
uint64_t hunksize = 0;

extern uint64_t hunk_low_used;
extern uint64_t hunk_high_used;

void Mem_Init(void);
void Z_Shutdown(void);

void Memory_Shutdown(void)
{
	Con_Printf("Memory_Shutdown: deallocating allocation daemons");
	Z_Shutdown();
    Mem_Shutdown();
}

void Zone_Stats(void);

static void Com_Meminfo_f(void)
{
	Con_Printf( "%8lu bytes total hunk", hunksize );
	Con_Printf( "%8lu bytes remaining", hunksize - hunk_low_used - hunk_low_used );
	Con_Printf( "%8p  hunk top address", (void *)(hunkbase + hunksize) );
	Con_Printf( "%8p  hunk bottom address", (void *)hunkbase );
	Con_Printf( " " );
	Con_Printf( "%8lu low mark", hunk_low_used );
	Con_Printf( "%8p  low used address", (void *)(hunkbase + hunk_low_used) );
	Con_Printf( "%8lu high mark", hunk_high_used );
	Con_Printf( "%8p  high used address", (void *)(hunkbase + hunksize - hunk_high_used));
	Con_Printf( " " );
	Zone_Stats();
}

uint64_t Com_TouchMemory(void)
{
	uint64_t i, j, sum, start, end;

	Z_CheckHeap();

	start = Sys_Milliseconds();
	sum = 0;

	j = hunk_low_used >> 2;
	for ( i = 0 ; i < j ; i+=64 ) {			// only need to touch each page
		sum += ((uint32_t *)hunkbase)[i];
	}

	i = ( hunksize - hunk_high_used ) >> 2;
	j = hunk_high_used >> 2;
	for (  ; i < j ; i+=64 ) {			// only need to touch each page
		sum += ((uint32_t *)hunkbase)[i];
	}

	Z_TouchMemory(&sum);

	end = Sys_Milliseconds();

	Con_Printf("Com_TouchMemory: %lu msec", end - start);

	return sum;
}

static void Com_TouchMemory_f(void)
{
	Com_TouchMemory();
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

	Mem_Init();
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
	
    hunkbase = (byte *)memset(malloc(hunksize), 0, hunksize);
    if (!hunkbase) {
        N_Error("Memory_Init: malloc() failed on %lu bytes when allocating the hunk", hunksize);
	}
	hunkbase = PADP(hunkbase, 64);
	// initialize it all
	Hunk_Clear();

	Con_Printf("Initialized heap from %p -> %p (%lu MiB)", (void *)hunkbase, (void *)(hunkbase + hunksize), hunksize >> 20);
	
	// initialize the zone heap
	Z_Init();

	Cmd_AddCommand("meminfo", Com_Meminfo_f);
	Cmd_AddCommand("touchmemory", Com_TouchMemory_f);
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