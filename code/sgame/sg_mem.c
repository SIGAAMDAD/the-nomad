#include "sg_local.h"

// 1 MiB of static memory for the vm to use
#define MEMPOOL_SIZE (1024*1024)
static char mempool[MEMPOOL_SIZE];
static uint32_t allocPoint;

void* SG_MemAlloc( uint32_t size)
{
    char *ptr;

    if (!size)
        return NULL;

    size = (size+15)&~15; // 16-bit alignment
    if (allocPoint + size >= MEMPOOL_SIZE) {
        SG_Error("SG_MemAlloc: failed to allocate %i bytes", size);
        return NULL;
    }

    ptr = &mempool[allocPoint];
    allocPoint += size;
    
    return ptr;
}

uint32_t SG_MemoryRemaining( void )
{
    return sizeof(mempool) - allocPoint;
}

void SG_ClearMem(void) {
    memset( mempool, 0, sizeof(mempool) );
    allocPoint = 0;
}
