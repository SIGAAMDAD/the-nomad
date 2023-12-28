#include "sg_local.h"

#define IDENT (('d'<<24)+('g'<<16)+('n'<<8)+'!')

typedef struct {
    int ident;
    int version;
    int versionUpdate;
    int versionPatch;
    int numSections;
} ngdheader_t;

typedef struct {
    char name[4];
    int size;
    int offset;
} ngdsection_t;

static ngdsection_t *sections;
static int numSections;
static file_t gamefile;

typedef struct archiveHandle_s {
    archiveFunc_t fn;
    struct archiveHandle_s *next;
    struct archiveHandle_s *prev;
} archiveHandle_t;

static archiveHandle_t s_archiveHandleList;

void SG_AddArchiveHandle( archiveFunc_t pFunc )
{
    archiveHandle_t *handle;

    // check if we already have it
    for ( handle = &s_archiveHandleList; handle; handle = handle->next ) {
        if ( handle->fn == pFunc ) {
            return;
        }
    }

    // allocate it on permanent memory
    handle = SG_MemAlloc( sizeof(*handle) );
    handle->fn = pFunc;

    // link into the list
	handle->next = s_archiveHandleList.next;
	handle->prev = &s_archiveHandleList;
	s_archiveHandleList.next->prev = handle;
	s_archiveHandleList.next = handle;
}

int SG_SaveGame( void )
{
    archiveHandle_t *handle;

    gamefile = trap_FS_FOpenWrite( sg_savename.s );
    if ( gamefile == FS_INVALID_HANDLE ) {
        G_Printf( COLOR_RED "Failed to create savefile titled '%s'!\n", sg_savename.s );
        return 0;
    }

    for ( handle = &s_archiveHandleList; handle; handle = handle->next ) {
        handle->fn( gamefile );
    }

    trap_FS_FClose( gamefile );
    
    return 1;
}

void SG_WriteSection( const char *name, int size, const void *data, file_t f )
{
    ngdsection_t section;

    if ( strlen( name ) >= sizeof(section.name) ) {
        trap_Error( "SG_WriteSection: strlen( name ) >= sizeof(section.name)" );
    }

    strcpy( section.name, name );
    section.size = size;

    trap_FS_Write( &section, sizeof(section), f );
    trap_FS_Write( data, size, f );
}

void SG_LoadSection( const char *name, void *dest, int size )
{
    int i;

    for ( i = 0; i < numSections; i++ ) {
        if ( !N_stricmp( name, sections[i].name ) ) {
            if ( sections[i].size != size ) {
                trap_Error( "SG_LoadSection: section size in gamefile != size in vm." );
            }

            trap_FS_FileSeek( gamefile, sections[i].offset, FS_SEEK_SET );
            trap_FS_Read( dest, size, gamefile );
        }
    }
    G_Error( "SG_LoadGame: failed to load section '%s'", name );
}

int SG_LoadGame( void )
{
    ngdheader_t header;
    archiveHandle_t *handle;
    sgameState_t statePrev;
    int i;
    int mark;

    mark = SG_MakeMemoryMark();

    gamefile = trap_FS_FOpenRead( sg_savename.s );
    if ( gamefile == FS_INVALID_HANDLE ) {
        G_Printf( COLOR_RED "Failed to load savefile titled '%s'!\n", sg_savename.s );
        return 0;
    }

    if ( !trap_FS_Read( &header, sizeof(header), gamefile ) ) {
        G_Printf( COLOR_RED "SG_LoadGame: failed to read savefile header.\n" );
        return 0;
    }

    if ( header.ident != IDENT ) {
        trap_FS_FClose( gamefile );
        trap_Error( "SG_LoadGame: savefile header identifier is incorrect" );
    }

    if ( header.version != NOMAD_VERSION || header.versionUpdate != NOMAD_VERSION_UPDATE || header.versionPatch != NOMAD_VERSION_PATCH ) {
        G_Printf( "SG_LoadGame: header version is not equal to this game's version.\n" );
        goto error;
    }

    if ( !header.numSections ) {
        G_Printf( "SG_LoadGame: header has invalid section count" );
        goto error;
    }

    statePrev = sg.state;
    sg.state = SG_LOADGAME;

    sections = (ngdsection_t *)SG_MemAlloc( sizeof(ngdsection_t) * header.numSections );
    numSections = header.numSections;

    for ( i = 0; i < header.numSections; i++ ) {
        // read info and go to the next section
        trap_FS_Read( sections[i].name, sizeof(sections->name), gamefile );
        trap_FS_Read( &sections[i].size, sizeof(sections->size), gamefile );

        sections[i].offset = trap_FS_FileTell( gamefile );
        trap_FS_FileSeek( gamefile, trap_FS_FileTell( gamefile ) + sections[i].size, FS_SEEK_SET );
    }

    for ( handle = &s_archiveHandleList; handle; handle = handle->next ) {
        handle->fn( gamefile );
    }

    trap_FS_FClose( gamefile );
    gamefile = FS_INVALID_HANDLE;

    return 1;

error:
    sg.state = statePrev;
    if ( gamefile != FS_INVALID_HANDLE ) {
        trap_FS_FClose( gamefile );
        gamefile = FS_INVALID_HANDLE;
    }

    return 0;
}
