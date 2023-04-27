#include "n_shared.h"
#include "n_alloc.h"

#define	HUNK_SENTINAL	0x1df001ed

typedef struct
{
	int		sentinal;
	size_t	size;		// including sizeof(hunk_t), -1 = not allocated
	char	name[14];
} hunk_t;

byte* hunk_base;
size_t hunk_size;

size_t hunk_low_used;
size_t hunk_high_used;

bool hunk_tempactive;
size_t hunk_tempmark;

/*
==============
Hunk_Check

Run consistancy and sentinal trahing checks
==============
*/
void Hunk_Check(void)
{
	hunk_t	*h;
	
	for (h = (hunk_t *)hunk_base; (byte *)h != hunk_base + hunk_low_used;) {
		if (h->sentinal != HUNK_SENTINAL)
			N_Error ("Hunk_Check: trahsed sentinal");
		if (h->size < 16 || h->size + (byte *)h - hunk_base > hunk_size)
			N_Error ("Hunk_Check: bad size");
		h = (hunk_t *)((byte *)h+h->size);
	}
}

/*
==============
Hunk_Print

If "all" is specified, every single allocation is printed.
Otherwise, allocations with the same name will be totaled up before printing.
==============
*/
void Hunk_Print(bool all)
{
	hunk_t	*h, *next, *endlow, *starthigh, *endhigh;
	size_t count, sum;
	size_t totalblocks;
	char	name[15];

	name[14] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	h = (hunk_t *)hunk_base;
	endlow = (hunk_t *)(hunk_base + hunk_low_used);
	starthigh = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);
	endhigh = (hunk_t *)(hunk_base + hunk_size);

	fprintf(stdout, "          :%8li total hunk size\n", hunk_size);
	fprintf(stdout, "-------------------------\n");

	while (1) {
		// skip to the high hunk if done with low hunk
		if (h == endlow) {
			fprintf(stdout, "-------------------------\n");
			fprintf(stdout, "          :%8li REMAINING\n", hunk_size - hunk_low_used - hunk_high_used);
			fprintf(stdout, "-------------------------\n");
			h = starthigh;
		}
		
		// if totally done, break
		if (h == endhigh)
			break;
		
		// run consistancy checks
		if (h->sentinal != HUNK_SENTINAL)
			N_Error("Hunk_Check: trahsed sentinal");
		if (h->size < 16 || size_t(h->size + (byte *)h - hunk_base) > hunk_size)
			N_Error("Hunk_Check: bad size");
			
		next = (hunk_t *)((byte *)h+h->size);
		count++;
		totalblocks++;
		sum += h->size;
		
		// print the single block
		memcpy(name, h->name, 8);
		if (all)
			fprintf(stdout, "%8p :%8li %14s\n", (void *)h, h->size, name);
		
		// print the total
		if (next == endlow || next == endhigh ||  strncmp(h->name, next->name, 8)) {
			if (!all)
				fprintf(stdout, "          :%8li %14s (TOTAL)\n", sum, name);
			
			count = 0;
			sum = 0;
		}

		h = next;
	}

	fprintf(stdout, "-------------------------\n");
	fprintf(stdout, "%8li total blocks\n", totalblocks);
	fflush(stdout);
}

/*
===================
Hunk_AllocName
===================
*/
void *Hunk_AllocName(size_t size, const char* name)
{
	hunk_t	*h;
#ifdef CHECKHEAP
	Hunk_Check();
#endif

	if (!size)
		N_Error("Hunk_Alloc: bad size: %li, name: %s", size, name);
		
	size = sizeof(hunk_t) + ((size+15)&~15);
	
	if (hunk_size - hunk_low_used - hunk_high_used < size)
		N_Error("Hunk_Alloc: failed on %li bytes, name: %s", size, name);
	
	h = (hunk_t *)(hunk_base + hunk_low_used);
	hunk_low_used += size;
	
	Cache_FreeLow(hunk_low_used);

	memset(h, 0, size);
	
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	strncpy(h->name, name, 14);
	
	return (void *)(h+1);
}

/*
===================
Hunk_Alloc
===================
*/
void *Hunk_Alloc(size_t size)
{
	return Hunk_AllocName(size, "unknown");
}

int	Hunk_LowMark(void)
{
	return hunk_low_used;
}

void Hunk_FreeToLowMark(size_t mark)
{
	if (mark < 0 || mark > hunk_low_used)
		N_Error("Hunk_FreeToLowMark: bad mark %li", mark);
	
	memset(hunk_base + mark, 0, hunk_low_used - mark);
	hunk_low_used = mark;
}

int	Hunk_HighMark(void)
{
	if (hunk_tempactive) {
		hunk_tempactive = false;
		Hunk_FreeToHighMark(hunk_tempmark);
	}

	return hunk_high_used;
}

void Hunk_FreeToHighMark(size_t mark)
{
	if (hunk_tempactive) {
		hunk_tempactive = false;
		Hunk_FreeToHighMark(hunk_tempmark);
	}
	if (mark < 0 || mark > hunk_high_used)
		N_Error("Hunk_FreeToHighMark: bad mark %li", mark);
	
	memset(hunk_base + hunk_size - hunk_high_used, 0, hunk_high_used - mark);
	hunk_high_used = mark;
}


/*
===================
Hunk_HighAllocName
===================
*/
void *Hunk_HighAllocName(size_t size, const char* name)
{
	hunk_t	*h;

	if (!size)
		N_Error("Hunk_HighAllocName: bad size: %li, name: %s", size, name);

	if (hunk_tempactive) {
		Hunk_FreeToHighMark(hunk_tempmark);
		hunk_tempactive = false;
	}

#ifdef CHECKHEAP
	Hunk_Check();
#endif

	size = sizeof(hunk_t) + ((size+15)&~15);

	if (hunk_size - hunk_low_used - hunk_high_used < size) {
		con.ConError("Hunk_HighAlloc: failed on {} bytes, name: {}", size, name);
		return NULL;
	}

	hunk_high_used += size;
	Cache_FreeHigh(hunk_high_used);

	h = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);

	memset(h, 0, size);
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	strncpy(h->name, name, 14);

	return (void *)(h+1);
}


/*
=================
Hunk_TempAlloc

Return space from the top of the hunk
=================
*/
void *Hunk_TempAlloc(size_t size)
{
	void *buf;
	size = (size+15)&~15;
	
	if (hunk_tempactive) {
		Hunk_FreeToHighMark(hunk_tempmark);
		hunk_tempactive = false;
	}
	hunk_tempmark = Hunk_HighMark();
	buf = Hunk_HighAllocName(size, "temp");
	hunk_tempactive = true;

	return buf;
}

//============================================================================


/*
========================
Memory_Init
========================
*/
#define DEFAULT_SIZE (34*1024) // 34 kb for strings and stuff like that
void Memory_Init(void *buf, size_t size)
{
	int p;
	uint32_t zonesize = DEFAULT_SIZE;
	
	hunk_base = (byte *)buf;
	hunk_size = size;
	hunk_low_used = 0;
	hunk_high_used = 0;
	
	Cache_Init();
	p = I_GetParm("-zoneram");
	if (p != -1) {
		if (p < myargc - 1)
			zonesize = atoi(myargv[p+1]) * 1024 * 1024;
		else
			N_Error("Memory_Init: you must specify a size in MB after -zoneram");
	}
	p = I_GetParm("-ram");
	if (p != -1) {
		if (p < myargc - 1)
			hunk_size = atoi(myargv[p+1]) * 1024 * 1024;
		else
			N_Error("Memory_Init: you must specify a size in MB after -ram");
	}
	mainzone = (memzone_t *)Hunk_AllocName(zonesize, "zone");
	Z_ClearZone (mainzone, zonesize);
}

typedef struct cache_system_s
{
	size_t					size;		// including this header
	cache_user_t			*user;
	char					name[15];
	struct cache_system_s	*prev, *next;
	struct cache_system_s	*lru_prev, *lru_next;	// for LRU flushing	
} cache_system_t;

cache_system_t	cache_head;

cache_system_t *Cache_TryAlloc(size_t size, bool nobottom);

/*
===========
Cache_Move
===========
*/
void Cache_Move(cache_system_t *c)
{
	cache_system_t		*newcache;

	// we are clearing up space at the bottom, so only allocate it late
	newcache = Cache_TryAlloc(c->size, true);
	if (newcache) {
		LOG_INFO("cache_move ok");
		memcpy(newcache+1, c+1, c->size - sizeof(cache_system_t));
		newcache->user = c->user;
		memcpy(newcache->name, c->name, sizeof(newcache->name));
		Cache_Free(c->user);
		*newcache->user = (void *)(newcache+1);
	}
	else {
		LOG_INFO("cache_move failed");
		Cache_Free(c->user);		// tough luck...
	}
}

/*
============
Cache_FreeLow

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeLow(size_t new_low_hunk)
{
	cache_system_t	*c;
	
	while (1) {
		c = cache_head.next;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ((byte *)c >= hunk_base + new_low_hunk)
			return;		// there is space to grow the hunk
		Cache_Move(c);	// reclaim the space
	}
}

/*
============
Cache_FreeHigh

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeHigh(size_t new_high_hunk)
{
	cache_system_t	*c, *prev;
	
	prev = NULL;
	while (1) {
		c = cache_head.prev;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ( (byte *)c + c->size <= hunk_base + hunk_size - new_high_hunk)
			return;		// there is space to grow the hunk
		if (c == prev)
			Cache_Free(c->user);	// didn't move out of the way
		else {
			Cache_Move (c);	// try to move it
			prev = c;
		}
	}
}

void Cache_UnlinkLRU (cache_system_t *cs)
{
	if (!cs->lru_next || !cs->lru_prev)
		N_Error("Cache_UnlinkLRU: NULL link");

	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;
	
	cs->lru_prev = cs->lru_next = NULL;
}

void Cache_MakeLRU(cache_system_t *cs)
{
	if (cs->lru_next || cs->lru_prev)
		N_Error("Cache_MakeLRU: active link");

	cache_head.lru_next->lru_prev = cs;
	cs->lru_next = cache_head.lru_next;
	cs->lru_prev = &cache_head;
	cache_head.lru_next = cs;
}

/*
============
Cache_TryAlloc

Looks for a free block of memory between the high and low hunk marks
Size should already include the header and padding
============
*/
cache_system_t *Cache_TryAlloc(size_t size, bool nobottom)
{
	cache_system_t	*cs, *newcache;
	
	// is the cache completely empty?
	if (!nobottom && cache_head.prev == &cache_head) {
		if (hunk_size - hunk_high_used - hunk_low_used < size)
			N_Error("Cache_TryAlloc: %li is greater then free hunk", size);

		newcache = (cache_system_t *)(hunk_base + hunk_low_used);
		memset(newcache, 0, sizeof(*newcache));
		newcache->size = size;
		
		cache_head.prev = cache_head.next = newcache;
		newcache->prev = newcache->next = &cache_head;
		
		Cache_MakeLRU(newcache);
		return newcache;
	}
	
	// search from the bottom up for space
	newcache = (cache_system_t *) (hunk_base + hunk_low_used);
	cs = cache_head.next;
	
	do {
		if (!nobottom || cs != cache_head.next) {
			if ( (byte *)cs - (byte *)newcache >= size) {
				// found space
				memset(newcache, 0, sizeof(*newcache));
				newcache->size = size;
				
				newcache->next = cs;
				newcache->prev = cs->prev;
				cs->prev->next = newcache;
				cs->prev = newcache;
				
				Cache_MakeLRU(newcache);
	
				return newcache;
			}
		}

		// continue looking		
		newcache = (cache_system_t *)((byte *)cs + cs->size);
		cs = cs->next;

	} while (cs != &cache_head);
	
	// try to allocate one at the very end
	if (hunk_base + hunk_size - hunk_high_used - (byte *)newcache >= size) {
		memset(newcache, 0, sizeof(*newcache));
		newcache->size = size;
		
		newcache->next = &cache_head;
		newcache->prev = cache_head.prev;
		cache_head.prev->next = newcache;
		cache_head.prev = newcache;
		
		Cache_MakeLRU(newcache);
		
		return newcache;
	}
	
	return NULL;		// couldn't allocate
}

/*
============
Cache_Flush

Throw everything out, so new data will be demand cached
============
*/
void Cache_Flush(void)
{
	while (cache_head.next != &cache_head)
		Cache_Free(cache_head.next->user); // reclaim the space
}


/*
============
Cache_Print

============
*/
void Cache_Print(void)
{
	cache_system_t	*cd;
	
	for (cd = cache_head.next ; cd != &cache_head ; cd = cd->next)
		fprintf(stdout, "%8li : %s\n", cd->size, cd->name);
}

/*
============
Cache_Report

============
*/
void Cache_Report(void)
{
	fprintf(stdout, "%4.1f / %li megabyte data cache\n", (hunk_size - hunk_high_used - hunk_low_used) / (float)(1024*1024),
		(hunk_size - hunk_high_used - hunk_low_used) >> 20);
}

/*
============
Cache_Compact

============
*/
void Cache_Compact(void)
{
}

/*
============
Cache_Init

============
*/
void Cache_Init(void)
{
	cache_head.next = cache_head.prev = &cache_head;
	cache_head.lru_next = cache_head.lru_prev = &cache_head;

//	Cmd_AddCommand ("flush", Cache_Flush);
}

/*
==============
Cache_Free

Frees the memory and removes it from the LRU list
==============
*/
void Cache_Free(cache_user_t *c)
{
	cache_system_t	*cs;

	if (!(*c))
		N_Error ("Cache_Free: not allocated");

	cs = ((cache_system_t *)(*c)) - 1;

	cs->prev->next = cs->next;
	cs->next->prev = cs->prev;
	cs->next = cs->prev = NULL;

	*c = NULL;

	Cache_UnlinkLRU (cs);
}


/*
==============
Cache_Check
==============
*/
void *Cache_Check(cache_user_t *c)
{
	cache_system_t	*cs;
	if (!(*c))
		return NULL;
	
	cs = ((cache_system_t *)(*c)) - 1;
	
	// move to head of LRU
	Cache_UnlinkLRU(cs);
	Cache_MakeLRU(cs);
	
	return *c;
}


/*
==============
Cache_Alloc
==============
*/
void *Cache_Alloc (cache_user_t *c, size_t size, const char* name)
{
	cache_system_t *cs;
	if (*c)
		N_Error ("Cache_Alloc: already allocated, name: %s", name);
	
	if (size <= 0)
		N_Error ("Cache_Alloc: size %li, name: %s", size, name);
	
	size = (size + sizeof(cache_system_t) + 15) & ~15;

	// find memory for it	
	while (1) {
		cs = Cache_TryAlloc(size, false);
		if (cs) {
			strncpy(cs->name, name, sizeof(cs->name)-1);
			*c = (void *)(cs+1);
			cs->user = c;
			break;
		}
		
		// free the least recently used cahedat
		if (cache_head.lru_prev == &cache_head)
			N_Error("Cache_Alloc: out of memory"); // not enough memory at all
		
		Cache_Free(cache_head.lru_prev->user);
	}
	
	return Cache_Check(c);
}

void *Cache_Realloc(cache_user_t *c, size_t nsize, const char* name)
{
	if (!c)
		N_Error("Cache_Realloc: pointer is null");
	if (nsize == 0)
		N_Error("Cache_Realloc: size is 0");
	
	cache_user_t user;
	void *p = Cache_Alloc(&user, nsize, name);
	if (*c) {
		cache_system_t* cs = ((cache_system_t*)(*c)) - 1;
		memcpy(user, *c, cs->size);
		Cache_Free(c);
	}
	return p;
}

void *Cache_Calloc(cache_user_t *c, size_t elemsize, size_t nelem, const char* name)
{
	return memset((Cache_Alloc)(c, elemsize * nelem, name), 0, elemsize * nelem);
}