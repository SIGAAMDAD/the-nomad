#include "sg_local.h"

// 256 KiB of static memory for the vm to use
#define MEMPOOL_SIZE (256*1024)
static char mempool[MEMPOOL_SIZE];
static uint32_t allocPoint;

void* SG_AllocMem( uint32_t size)
{
    char *ptr;

    if (!size)
        return NULL;

    size = (size+15)&~15; // 16-bit alignment
    if (allocPoint + size >= MEMPOOL_SIZE) {
        SG_Error("SG_AllocMem: failed to allocate %i bytes", size);
        return NULL;
    }

    ptr = &mempool[allocPoint];
    allocPoint += size;
    
    return ptr;
}

void SG_ClearMem(void) {
    memset( mempool, 0, sizeof(mempool) );
    allocPoint = 0;
}
