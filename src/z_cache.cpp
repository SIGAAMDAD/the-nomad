#include "n_shared.h"
#include "n_alloc.h"

typedef struct cache_system_s
{
	size_t					size;		// including this header
	cache_user_t			*user;
	char					name[15];
	struct cache_system_s	*prev, *next;
	struct cache_system_s	*lru_prev, *lru_next;	// for LRU flushing	
} cache_system_t;

cache_system_t *Cache_TryAlloc(int32_t size, bool nobottom);

cache_system_t	cache_head;

void __attribute__((format (printf, 1, 2))) Con_Printf(const char* fmt, ...);
void __attribute__((format (printf, 1, 2))) Con_Error(const char* fmt, ...);
void __attribute__((format (printf, 1, 2))) Con_Debug(const char* fmt, ...);

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
		newcache->user->data = (void *)(newcache+1);
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
		
		return ;
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

	if (!c->data)
		N_Error ("Cache_Free: not allocated");

	cs = ((cache_system_t *)c->data) - 1;

	cs->prev->next = cs->next;
	cs->next->prev = cs->prev;
	cs->next = cs->prev = NULL;

	c->data = NULL;

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
	if (!c->data)
		return NULL;
	
	cs = ((cache_system_t *)c->data) - 1;
	
	// move to head of LRU
	Cache_UnlinkLRU(cs);
	Cache_MakeLRU(cs);
	
	return c->data;
}


/*
==============
Cache_Alloc
==============
*/
void *Cache_Alloc (cache_user_t *c, size_t size, const char* name)
{
	cache_system_t *cs;
	if (c->data)
		N_Error ("Cache_Alloc: allready allocated, name: %s", name);
	
	if (size <= 0)
		N_Error ("Cache_Alloc: size %li, name: %s", size, name);
	
	size = (size + sizeof(cache_system_t) + 15) & ~15;

	// find memory for it	
	while (1) {
		cs = Cache_TryAlloc(size, false);
		if (cs) {
			strncpy(cs->name, name, sizeof(cs->name)-1);
			c->data = (void *)(cs+1);
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
	if (c->data) {
		cache_system_t* cs = ((cache_system_t*)c->data) - 1;
		memcpy(user.data, c->data, cs->size);
		Cache_Free(c);
	}
	return p;
}

void *Cache_Calloc(cache_user_t *c, size_t elemsize, size_t nelem, const char* name)
{
	return memset((Cache_Alloc)(c, elemsize * nelem, name), 0, elemsize * nelem);
}