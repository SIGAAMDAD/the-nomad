#include "sg_local.h"

// 256 KiB of static memory for the vm to use
#define MEMPOOL_SIZE (256*1024)
static char mempool[MEMPOOL_SIZE];
static int allocPoint;

void SG_InitMem(void)
{
    allocPoint = 0;
    memset(mempool, 0, sizeof(mempool));
}

void* SG_AllocMem(int size)
{
    char *ptr;

    if (!size)
        return NULL;

    size += sizeof(int);
    size = (size + 31) & ~31;
    if (allocPoint + size >= MEMPOOL_SIZE) {
        SG_Error("SG_AllocMem: failed to allocate %i bytes", size);
        return NULL;
    }
    allocPoint += size;
    ptr = &mempool[allocPoint];
    *(int *)ptr = size;
    return (void *)(ptr + sizeof(int));
}

void SG_FreeMem(void *ptr)
{
    int size;
    if (ptr == NULL) {
        SG_Error("SG_FreeMem: null pointer");
        return;
    }

    size = *(int *)((char *)ptr - sizeof(int));
    allocPoint -= size;
}

void SG_ClearMem(void)
{
    SG_InitMem();
}
