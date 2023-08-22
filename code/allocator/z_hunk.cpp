#include "../engine/n_shared.h"

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
#define HUNKFREE 0x0ffa3d

typedef struct hunkblock_s
{
	uint64_t id;
	uint64_t size;
	struct hunkblock_s *next;
	const char *name;
} hunkblock_t;

typedef struct
{
	uint64_t id;
	uint64_t size;
} hunkHeader_t;

static hunkblock_t *hunkblocks;

typedef struct
{
	uint64_t mark;			// temp + permanent
	uint64_t permanent;		// permanent memory (can only be reset after game restart)
	uint64_t temp;			// temp memory (can be deallocated at any time)
	uint64_t highWater;
} hunkUsed_t;

boost::recursive_mutex hunkLock, allocLock;
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
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};

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
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};
	hunk_low.mark = hunk_low.permanent;
	hunk_high.mark = hunk_high.permanent;
}

void Hunk_ClearToMark( void )
{
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};
	hunk_low.permanent = hunk_low.temp = hunk_low.mark;
	hunk_high.permanent = hunk_high.temp = hunk_high.mark;
}

// broken
#if 0
/*
Hunk_Check: Run consistancy and id checks
*/
void Hunk_Check (void)
{
	hunkblock_t *h;
	
	for (h = hunkblocks; h; h = h->next) {
		if (h->id != HUNKID)
			N_Error ("Hunk_Check: hunk id isn't correct");
		if (h->size < 16 || h->size + (byte *)h - hunkbase > hunksize)
			N_Error ("Hunk_Check: bad size");
	}
}
#endif

static void Hunk_SwapBanks(void)
{
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};
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

void Hunk_Stats(void)
{
	uint64_t unused;

	Con_Printf( "%8lu bytes total hunk", hunksize );
	Con_Printf( " " );
	Con_Printf( "%8lu low mark", hunk_low.mark );
	Con_Printf( "%8lu low permanent", hunk_low.permanent );
	if ( hunk_low.temp != hunk_low.permanent ) {
		Con_Printf( "%8lu low temp", hunk_low.temp );
	}
	Con_Printf( "%8lu low highWater", hunk_low.highWater );
	Con_Printf( " " );
	Con_Printf( "%8lu high mark", hunk_high.mark );
	Con_Printf( "%8lu high permanent", hunk_high.permanent );
	if ( hunk_high.temp != hunk_high.permanent ) {
		Con_Printf( "%8lu high temp", hunk_high.temp );
	}
	Con_Printf( "%8lu high highWater", hunk_high.highWater );
	Con_Printf( " " );
	Con_Printf( "%8lu total hunk in use", hunk_low.permanent + hunk_high.permanent );

	unused = 0;
	if ( hunk_low.highWater > hunk_low.permanent ) {
		unused += hunk_low.highWater - hunk_low.permanent;
	}
	if ( hunk_high.highWater > hunk_high.permanent ) {
		unused += hunk_high.highWater - hunk_high.permanent;
	}
	Con_Printf( "%8lu unused highWater", unused );
	Con_Printf( " " );
}


void *Hunk_AllocateTempMemory(uint64_t size)
{
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};
	void *buf;
	hunkHeader_t *h;

	// if the hunk hasn't been initialized yet, but there are filesystem calls
	// being made, then just allocate with Z_Malloc
	if (hunkbase == NULL) {
		return Z_Malloc(size, TAG_HUNK, NULL, "temp");
	}

	Hunk_SwapBanks();
	
	size = PAD(size, sizeof(intptr_t)) + sizeof(hunkHeader_t);

	if (hunk_temp->temp + hunk_permanent->permanent + size > hunksize) {
		Hunk_Stats();
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

	h = (hunkHeader_t *)buf;
	buf = (void *)(h+1);

	h->id = HUNKID;
	h->size = size;

	// don't bother clearing, because we are going to load a file over it
	return buf;
}

void *Hunk_ReallocateTempMemory(void *ptr, uint64_t nsize)
{
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};
	hunkHeader_t *h;
	void *p;

	// if hunk hasn't been initialized yet, just Z_Realloc it
	if (hunkbase == NULL) {
		return Z_Realloc(ptr, nsize, TAG_HUNK, NULL, "temp");
	}

	if (ptr) {
		h = ((hunkHeader_t *)ptr) - 1;
		Hunk_FreeTempMemory(ptr);
		p = Hunk_AllocateTempMemory(nsize);
		memcpy(p, ptr, h->size);
	}
	else {
		p = Hunk_AllocateTempMemory(nsize);
	}
	return p;
}

void Hunk_FreeTempMemory(void *p)
{
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};
	hunkHeader_t *h;

	// if hunk hasn't been initialized yet, just Z_Free the data
	if (hunkbase == NULL) {
		Z_Free(p);
		return;
	}

	h = ((hunkHeader_t *)p) - 1;
	if (h->id != HUNKID) {
		N_Error ("Hunk_FreeTempMemory: hunk id isn't correct");
	}

	h->id = HUNKFREE;

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
	boost::lock_guard<boost::recursive_mutex> lock{hunkLock};
	if (hunkbase) {
		hunk_temp->temp = hunk_temp->permanent;
	}
}

qboolean Hunk_TempIsClear(void)
{
	return (qboolean)(hunk_temp->temp == hunk_temp->permanent);
}

qboolean Hunk_CheckMark(void)
{
	if (hunk_low.mark || hunk_high.mark)
		return qtrue;
	
	return qfalse;
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
	boost::lock_guard<boost::recursive_mutex> lock{allocLock};
	void *buf;

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

#ifdef _NOMAD_DEBUG
	size += sizeof(hunkblock_t);
#endif

	// round to the cacheline
	size = PAD(size, 64);

	if ( hunk_low.temp + hunk_high.temp + size > hunksize ) {
#ifdef _NOMAD_DEBUG
		Hunk_Log();
		Hunk_SmallLog();

		Con_Printf(ERROR, "Hunk_Alloc failed on %lu: %s, line: %lu (%s)", size, file, line, name);
#else
		Con_Printf(ERROR, "Hunk_Alloc failed on %lu", size);
#endif
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

#ifdef _NOMAD_DEBUG
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
#endif

	return buf;
}

void Hunk_Print(void)
{
    hunkblock_t *h, *prev;
	uint64_t count, sum;
	uint64_t totalblocks;

	if (!hunkblocks)
		return;

	count = 0;
	sum = 0;
	totalblocks = 0;

	h = hunkblocks;
	prev = h;

    Con_Printf("\n\n<----- Hunk Heap Report ----->");
	Con_Printf("          :%8lu total hunk size", hunksize);
	Con_Printf("-------------------------");

	while (1) {
		prev = h;
		h = h->next;
		// we've scanned around the list
		if (!h)
			break;
		
		count++;
		totalblocks++;
		sum += h->size;

		// print the single block
		Con_Printf("%8p : %8lu   %8s", h, h->size, h->name);
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
