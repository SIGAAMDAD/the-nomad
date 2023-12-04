#include "sg_local.h"

typedef struct archiveHandle_s {
    archiveFunc_t fn;
    struct archiveHandle_s *next;
    struct archiveHandle_s *prev;
} archiveHandle_t;

static archiveHandle_t s_archiveHandleList;

void SG_AddArchiveHandle( archiveFunc_t pFunc )
{
    archiveHandle_t *handle;

    // allocate it on permanent memory
    handle = SG_MemAlloc( sizeof(*handle) );

    // link into the list
	handle->next = s_archiveHandleList.next;
	handle->prev = &s_archiveHandleList;
	s_archiveHandleList.next->prev = handle;
	s_archiveHandleList.next = handle;
}

void SG_SaveGame( void )
{
    archiveHandle_t *handle;
    file_t file;

    file = trap_FS_FOpenWrite( sg_savename.s );
    if (file == FS_INVALID_HANDLE) {
        G_Printf( COLOR_RED "Failed to create savefile titled '%s'!\n", sg_savename.s );
        return;
    }

    for (handle = s_archiveHandleList.next; handle; handle = handle->next) {
        handle->fn( file, ARCHIVE_SAVEGAME );
    }

    trap_FS_FClose( file );
}
