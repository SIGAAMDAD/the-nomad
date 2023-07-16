#include "n_shared.h"

/*
===============================
Hunk Allocator:
Only meant for large static allocations. Each allocation is not temporary, cannot be freed (no direct function to do it), and is expected to be allocated
for the entirety of runtime. ONLY MEANT FOR MAIN ENGINE ALLOCATIONS
===============================
*/

extern uint64_t hunksize;
extern byte* hunkbase;

#define HUNKID 0x553dfa2

typedef struct
{
	char name[14];
	uint64_t id;
	uint64_t size;
} hunk_t;

static uint64_t hunk_low_used = 0;
static uint64_t hunk_high_used = 0;
static uint64_t hunk_temp_used = 0;

void Hunk_Clear(void)
{
	hunk_high_used = 0;
	hunk_low_used = 0;
	memset(hunkbase, 0, hunksize);
}

uint64_t Hunk_MemoryRemaining (void)
{
	return hunksize - hunk_low_used - hunk_high_used;
}

/*
==============
Hunk_Check

Run consistancy and id checks
==============
*/
void Hunk_Check (void)
{
	hunk_t	*h;
	
	for (h = (hunk_t *)hunkbase ; (byte *)h != hunkbase + hunk_low_used ; ) {
		if (h->id != HUNKID)
			N_Error ("Hunk_Check: hunk id isn't correct");
		if (h->size < 16 || h->size + (byte *)h - hunkbase > hunksize)
			N_Error ("Hunk_Check: bad size");
		h = (hunk_t *)((byte *)h+h->size);
	}
}

/*
Hunk_HighAlloc: allocates memory on the high hunk temporary memory
*/
static void *Hunk_HighAlloc (uint64_t size, const char *name)
{
	hunk_t *h;

	hunk_high_used += size;
	h = (hunk_t *)(hunkbase + hunksize - hunk_high_used);

	h->size = size;
	h->id = HUNKID;
	N_strncpy(h->name, name, sizeof(h->name));

	return (void *)(h+1);
}

static void *Hunk_LowAlloc(uint64_t size, const char *name)
{
	hunk_t *h;

	hunk_low_used += size;
	h = (hunk_t *)(hunkbase + hunk_low_used);

	h->size = size;
	h->id = HUNKID;
	N_strncpy(h->name, name, sizeof(h->name));

	return (void *)(h+1);
}

void *Hunk_AllocateTempMemory(uint64_t size)
{
	hunk_t *h;

	// if the hunk hasn't been initialized yet, but there are filesystem calls
	// being made, then just allocate with Z_Malloc
	if (!hunkbase) {
		return Z_Malloc(size, TAG_STATIC, NULL, "temp");
	}

	size = sizeof(hunk_t) + ((size+15)&~15);

	if (hunksize - hunk_low_used - hunk_high_used < size) {
		Con_Printf(ERROR, "Hunk_AllocateTempMemory: failed on %lu", size);
		return NULL;
	}

	hunk_high_used += size;
	h = (hunk_t *)(hunkbase + hunksize - hunk_high_used);

	h->size = size;
	h->id = HUNKID;
	N_strncpy(h->name, "temp", sizeof(h->name));

	return (void *)(h+1);
}

void Hunk_FreeTempMemory(void *p)
{
	hunk_t *h;

	// if hunk hasn't been initialized yet, just Z_Free the data
	if (!hunkbase) {
		Z_Free(p);
		return;
	}

	h = ((hunk_t *)p) - 1;

	if (h->id != HUNKID) {
		N_Error ("Hunk_FreeTempMemory: hunk id isn't correct");
	}

	if (h == (hunk_t *)(hunkbase + hunksize - hunk_high_used))
		hunk_high_used -= h->size;
	else
		Con_Printf("Hunk_FreeTempMemory: not final block");
}

void Hunk_ClearTempMemory(void)
{
	if (hunkbase) {
		// set it all to 0
		memset(hunkbase + hunksize - hunk_high_used, 0, hunk_high_used);
		hunk_high_used = 0;
	}
}

uint64_t Hunk_LowMark(void)
{
	return hunk_low_used;
}

void Hunk_FreeToLowMark(uint64_t mark)
{
	if (mark > hunk_low_used)
		N_Error("Hunk_FreeToLowMark: bad mark");
	
	memset(hunkbase + mark, 0, hunk_low_used - mark);
	hunk_low_used = mark;
}

/*
Hunk_Alloc: allocates a chunk of memory out of the hunk heap in the ha_pref (h_low, h_high)
*/
#ifdef _NOMAD_DEBUG
void *Hunk_AllocDebug (uint64_t size, ha_pref where, const char *name, const char *file, uint64_t line)
#else
void *Hunk_Alloc (uint64_t size, const char *name, ha_pref where)
#endif
{
	hunk_t	*h;
	
#ifdef _NOMAD_DEBUG
	Hunk_Check ();
#endif

	if (hunksize - hunk_low_used - hunk_high_used < size)
		N_Error ("Hunk_Alloc: failed on %lu bytes", size);
	if (!size)
		N_Error ("Hunk_Alloc: bad size: %lu", size);

	size = sizeof(hunk_t) + ((size+15)&~15);
	
	h = (hunk_t *)(hunkbase + hunk_low_used);
	hunk_low_used += size;

	h->size = size;
	h->id = HUNKID;
	N_strncpy(h->name, name, sizeof(h->name));

	return (void *)(h+1);
}

extern byte *hunkbase;
extern uint64_t hunksize;

void Hunk_Print(void)
{
    hunk_t *h, *next, *endlow, *starthigh, *endhigh;
	uint64_t count, sum;
	uint64_t totalblocks;
	char name[15];

	name[14] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	h = (hunk_t *)hunkbase;
	endlow = (hunk_t *)(hunkbase + hunk_low_used);
	starthigh = (hunk_t *)(hunkbase + hunksize - hunk_high_used);
	endhigh = (hunk_t *)(hunkbase + hunksize);

    Con_Printf("\n\n<----- Hunk Heap Report ----->");
	Con_Printf("          :%8lu total hunk size", hunksize);
	Con_Printf("-------------------------");

	while (1) {
		// skip to the high hunk if done with low hunk
		if (h == endlow)
			h = starthigh;
		
		// if totally done, break
		if (h == endhigh)
			break;

		// run consistancy checks
		if (h->id != HUNKID)
			N_Error("Hunk_Check: hunk id isn't correct");
		if (h->size < 16 || h->size + (byte *)h - hunkbase > hunksize)
			N_Error("Hunk_Check: bad size");
			
		next = (hunk_t *)((byte *)h+h->size);
		count++;
		totalblocks++;
		sum += h->size;

		// print the single block
		memcpy(name, h->name, 14);
		Con_Printf("%8p : %8lu   %8s", h, h->size, name);

		h = next;
	}
	Con_Printf("-------------------------");
	Con_Printf("          :%8lu REMAINING", hunksize - hunk_low_used - hunk_high_used);
	Con_Printf("-------------------------");
	Con_Printf("          :%8lu (TOTAL)", sum);
	count = 0;
	sum = 0;

	Con_Printf("-------------------------");
	Con_Printf("%8lu total allocations\n\n", totalblocks);
}


void Hunk_SmallLog(void)
{
	hunk_t *h;
	char buf[4096];
	char name[15];
	uint64_t size, locsize, numBlocks;

	if (!logfile || !FS_Initialized())
		return;
	
	name[14] = '\0';
	size = 0;
	numBlocks = 0;
	stbsp_snprintf(buf, sizeof(buf), "\r\n================\r\nHunk Small log\r\n================\r\n");
	FS_Write(buf, strlen(buf), logfile);
	for (h = (hunk_t *)hunkbase; (byte *)h != hunkbase + hunk_low_used; h = (hunk_t *)((byte *)h+h->size)) {
		size += h->size;
		numBlocks++;
		memcpy(name, h->name, 14);
		stbsp_snprintf(buf, sizeof(buf), "size = %8lu: %s\r\n", h->size, name);
		FS_Write(buf, strlen(buf), logfile);
	}
	stbsp_snprintf(buf, sizeof(buf), "%lu total hunk memory\r\n", size);
	FS_Write(buf, strlen(buf), logfile);
	stbsp_snprintf(buf, sizeof(buf), "%lu total hunk blocks\r\n", numBlocks);
}

void Hunk_Log(void)
{
	Hunk_SmallLog();
}