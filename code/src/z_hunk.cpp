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

static cvar_t *z_hunkDebug;
static cvar_t *z_hunkMegs;

#define HUNKID 0x553dfa2

typedef struct hunkblock_s
{
	uint64_t id;
	uint64_t size;
	struct hunkblock_s *next;
	const char *name;
} hunkblock_t;

static hunkblock_t *hunkblocks;

typedef struct
{
	uint64_t mark;			// temp + permanent
	uint64_t permanent;		// permanent memory (can only be reset after game restart)
	uint64_t temp;			// temp memory (can be deallocated at any time)
	uint64_t highWater;
} hunkUsed_t;

static boost::shared_mutex hunkLock;
uint64_t hunk_low_used = 0;
uint64_t hunk_high_used = 0;
uint64_t hunk_temp_used = 0;

hunkUsed_t *hunk_permanent;
hunkUsed_t *hunk_temp;
hunkUsed_t hunk_low;
hunkUsed_t hunk_high;

/*
Hunk_Clear: gets called whenever a new level is loaded or is being shutdown
*/
void Hunk_Clear(void)
{	
	hunk_low.highWater = 0;
	hunk_low.mark = 0;
	hunk_low.temp = 0;
	hunk_low.permanent = 0;

	hunk_high.highWater = 0;
	hunk_high.mark = 0;
	hunk_high.temp = 0;
	hunk_high.permanent = 0;
	
	hunk_permanent = &hunk_low;
	hunk_temp = &hunk_high;

	Con_Printf("Hunk_Clear: reset the hunk ok");

	hunkblocks = NULL;
}

uint64_t Hunk_MemoryRemaining( void )
{
	uint64_t low, high;

	low = hunk_low.permanent > hunk_low.temp ? hunk_low.permanent : hunk_low.temp;
	high = hunk_high.permanent > hunk_high.temp ? hunk_high.permanent : hunk_high.temp;

	return hunksize - ( low + high );
}

/*
Hunk_SetMark: gets called after the level and game vm have been loaded
*/
void Hunk_SetMark( void )
{
	hunk_low.mark = hunk_low.permanent;
	hunk_high.mark = hunk_high.permanent;
}

void Hunk_ClearToMark( void )
{
	hunk_low.permanent = hunk_low.temp = hunk_low.mark;
	hunk_high.permanent = hunk_high.temp = hunk_high.mark;
}

/*
Hunk_Check: Run consistancy and id checks
*/
void Hunk_Check (void)
{
	hunkblock_t *h;
	
	for (h = hunkblocks; h; h = h->next) {
		if (h->id != HUNKID)
			N_Error ("Hunk_Check: hunk id isn't correct");
		if (h->size < 64 || h->size + (byte *)h - hunkbase > hunksize)
			N_Error ("Hunk_Check: bad size");
	}
}

static void Hunk_SwapBanks(void)
{
	hunkUsed_t	*swap;

	// can't swap banks if there is any temp already allocated
	if ( hunk_temp->temp != hunk_temp->permanent ) {
		return;
	}

	// if we have a larger highwater mark on this side, start making
	// our permanent allocations here and use the other side for temp
	if ( hunk_temp->highWater - hunk_temp->permanent >
		hunk_permanent->highWater - hunk_permanent->permanent ) {
		swap = hunk_temp;
		hunk_temp = hunk_permanent;
		hunk_permanent = swap;
	}
}


void *Hunk_AllocateTempMemory(uint64_t size)
{
	void *buf;
	hunkblock_t *h;

	// if the hunk hasn't been initialized yet, but there are filesystem calls
	// being made, then just allocate with Z_Malloc
	if (!hunkbase) {
		return Z_Malloc(size, TAG_STATIC, NULL, "temp");
	}

	Hunk_SwapBanks();
	
	size = PAD(size, sizeof(intptr_t) + sizeof(hunkblock_t));

	if (hunk_temp->temp + hunk_permanent->permanent + size > hunksize) {
		Con_Printf(ERROR, "Hunk_AllocateTempMemory: failed on %lu", size);
		return NULL;
	}

	if ( hunk_temp == &hunk_low ) {
		buf = (void *)(hunkbase + hunk_temp->temp);
		hunk_temp->temp += size;
	}
	else {
		hunk_temp->temp += size;
		buf = (void *)(hunkbase + hunksize - hunk_temp->temp );
	}

	if ( hunk_temp->temp > hunk_temp->highWater ) {
		hunk_temp->highWater = hunk_temp->temp;
	}

	h = (hunkblock_t *)buf;
	buf = (void *)(h+1);

	h->id = HUNKID;
	h->size = size;

	// don't bother clearing, because we are going to load a file over it
	return buf;
}

void Hunk_FreeTempMemory(void *p)
{
	hunkblock_t *h;

	// if hunk hasn't been initialized yet, just Z_Free the data
	if (!hunkbase) {
		Z_Free(p);
		return;
	}

	h = ((hunkblock_t *)p) - 1;

	if (h->id != HUNKID) {
		N_Error ("Hunk_FreeTempMemory: hunk id isn't correct");
	}

	// this only works if the files are freed in the stack order,
	// otherwise the memory will stay around until Hunk_ClearTempMemory
	if ( hunk_temp == &hunk_low ) {
		if ( h == (void *)(hunkbase + hunk_temp->temp - h->size ) )
			hunk_temp->temp -= h->size;
		else
			Con_Printf( "Hunk_FreeTempMemory: not the final block" );
	}
	else {
		if ( h == (void *)(hunkbase + hunksize - hunk_temp->temp ) )
			hunk_temp->temp -= h->size;
		else
			Con_Printf( "Hunk_FreeTempMemory: not the final block" );
	}
}

void Hunk_ClearTempMemory(void)
{
	if (hunkbase) {
		// set it all to 0
		memset(hunkbase + hunksize - hunk_high_used, 0, hunk_high_used);
		hunk_high_used = 0;
	}
}

qboolean Hunk_CheckMark(void)
{
	if (hunk_low.mark || hunk_high.mark)
		return qtrue;
	
	return qfalse;
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
	void *buf;
#ifdef _NOMAD_DEBUG
	Hunk_Check ();
#endif

	if (!size)
		N_Error("Hunk_Alloc: bad size");
	
	if (where == h_dontcare || hunk_temp->temp != hunk_temp->permanent) {
		Hunk_SwapBanks();
	}
	else {
		if (where == h_low && hunk_permanent != &hunk_low) {
			Hunk_SwapBanks();
		}
		else if (where == h_high && hunk_permanent != &hunk_high) {
			Hunk_SwapBanks();
		}
	}

	// round to the cacheline
	size = sizeof(hunkblock_t) + PAD(size, 64);

	if ( hunk_low.temp + hunk_high.temp + size > hunksize ) {
		N_Error("Hunk_Alloc failed on %li", size);
	}

	if ( hunk_permanent == &hunk_low ) {
		buf = (void *)(hunkbase + hunk_permanent->permanent);
		hunk_permanent->permanent += size;
	}
	else {
		hunk_permanent->permanent += size;
		buf = (void *)(hunkbase + hunksize - hunk_permanent->permanent );
	}

	hunk_permanent->temp = hunk_permanent->permanent;

	memset( buf, 0, size );

	{
		hunkblock_t *block;

		block = (hunkblock_t *)buf;
		block->size = size - sizeof(hunkblock_t);
		block->name = name;
		block->id = HUNKID;
		block->next = hunkblocks;
		hunkblocks = block;
		buf = ((byte *)buf) + sizeof(hunkblock_t);
	}

	return buf;
}

extern byte *hunkbase;
extern uint64_t hunksize;

void Hunk_Print(void)
{
    hunkblock_t *h, *prev;
	uint64_t count, sum;
	uint64_t totalblocks;

	count = 0;
	sum = 0;
	totalblocks = 0;

	h = hunkblocks;
	prev = h;

    Con_Printf("\n\n<----- Hunk Heap Report ----->");
	Con_Printf("          :%8lu total hunk size", hunksize);
	Con_Printf("-------------------------");

	while (1) {

		// run consistancy checks
		if (h->id != HUNKID)
			N_Error("Hunk_Check: hunk id isn't correct, prev name: '%s'", prev->name);
		if (h->size < 64 || h->size + (byte *)h - hunkbase > hunksize)
			N_Error("Hunk_Check: bad size, name: '%s'", h->name);
		
		h = h->next;
		count++;
		totalblocks++;
		sum += h->size;

		// print the single block
		Con_Printf("%8p : %8lu   %8s", h, h->size, h->name);

		prev = h;
		h = h->next;

		// we've scanned around the list
		if (!h)
			break;
	}
	Con_Printf("-------------------------");
	Con_Printf("          :%8lu REMAINING", Hunk_MemoryRemaining());
	Con_Printf("-------------------------");
	Con_Printf("          :%8lu (TOTAL)", sum);
	count = 0;
	sum = 0;

	Con_Printf("-------------------------");
	Con_Printf("%8lu total allocations\n\n", totalblocks);
}


void Hunk_SmallLog(void)
{
	hunkblock_t *h;
	char buf[4096];
	uint64_t size, locsize, numBlocks;

	if (!logfile || !FS_Initialized())
		return;
	
	size = 0;
	numBlocks = 0;
	stbsp_snprintf(buf, sizeof(buf), "\r\n================\r\nHunk Small log\r\n================\r\n");
	FS_Write(buf, strlen(buf), logfile);
	for (h = hunkblocks; h; h = h->next) {
		size += h->size;
		numBlocks++;
		stbsp_snprintf(buf, sizeof(buf), "size = %8lu: %s\r\n", h->size, h->name);
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