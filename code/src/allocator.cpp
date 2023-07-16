#include "n_shared.h"

#define HEAPID 0x424ae1

#define ALIGN_MEM( bytes, alignment ) ( ( (bytes) + alignment - 1 ) & ~(alignment - 1) )

//#define BIG_PAGE_SIZE 64*1024*1024
//#define MEDIUM_PAGE_SIZE 32*1024*1024
//#define SMALL_PAGE_SIZE 8*1024*1024
#define BIG_ALLOC_SIZE 64*1024
#define MEDIUM_ALLOC_SIZE 32*1024
#define SMALL_ALLOC_SIZE 8*1024

typedef struct page_s
{
	struct page_s* next;
	struct page_s* prev;
	
	void *cachePtr;
	uint32_t id;
	uint32_t pageSize;
} page_t;

page_t* pageList;
uint32_t numPages;

static uint64_t totalAlloced;
static uint64_t totalFreed;

void* Mem_Alloc(uint32_t size)
{
	if (!size) {
		N_Error("Mem_Alloc: bad size");
	}

	size_t alignment;

	if (size >= BIG_ALLOC_SIZE) {
		alignment = 64;
	}
	else if (size >= MEDIUM_ALLOC_SIZE) {
		alignment = 32;
	}
	else if (size >= SMALL_ALLOC_SIZE) {
		alignment = 16;
	}
	size = ALIGN_MEM(size, alignment);
	
	const size_t offset = alignment - 1 + sizeof(page_t);
	void *p = ::malloc(size + offset);
	if (!p) {
		N_Error("Mem_Alloc: malloc() failed");
	}
	void **p2 = (void**)(((size_t)(p) + offset) & ~(alignment - 1));

	page_t* page;
	for (page = pageList;; page = page->next) {
		if (!page->next) {
			break;
		}
	}
	page->next = (page_t *)((char *)p2 - sizeof(page_t));
	page->next->prev = page;
	page = page->next;

	page->id = HEAPID;
	page->cachePtr = p2;
	page->pageSize = size;

	totalAlloced += size;
	numPages++;

	return p2;
}

void Mem_Free(void *p)
{
	if (!p) {
		N_Error("Mem_Free: null pointer");
	}

	page_t* page = (page_t *)((char *)p - sizeof(page_t));

	if (page->id != HEAPID) {
		N_Error("Mem_Free: page->id isn't HEAPID");
	}
	::free(page->cachePtr);
}

void Mem_Init(void)
{
	pageList = new page_t;
	pageList->next = pageList->prev = NULL;
	pageList->pageSize = 0;
	pageList->cachePtr = NULL;
}

void Mem_Shutdown(void)
{
	for (page_t* p = pageList->next; p; p = p->next) {
		if (!p)
			break;
		
		::free(p->cachePtr);
	}
	delete pageList;
}