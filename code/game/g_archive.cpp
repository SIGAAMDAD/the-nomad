#include "g_game.h"
#include "g_archive.h"

#define NGD_MAGIC 0xff5ad1120
#define XOR_MAGIC 0xff

#define IDENT (('d'<<24)+('g'<<16)+('n'<<8)+'!')

typedef struct {
    int32_t ident;

    // version, 64 bits
    uint16_t versionMajor;
    uint16_t versionUpdate;
    uint32_t versionPatch;

    uint32_t numSections;

    gamedata_t gamedata;
} ngdheader_t;

typedef struct {
    char name[4];
    uint32_t size;
    uint32_t offset;
} ngdsection_t;

CGameArchive::CGameArchive( void ) {
}

CGameArchive::~CGameArchive() {
}

static void ReadBytes( void *dest, uint64_t size, file_t f )
{
    byte *p = (byte *)dest;

    FS_Read( dest, size, f );

    while (size--) {
        *p++ = *p ^ XOR_MAGIC;
    }
}

static void WriteBytes( const void *src, uint64_t size, file_t f )
{
    byte *p = const_cast<byte *>((const byte *)src);
    const uint64_t len = size;

    while (size--) {
        *p++ = *p ^ XOR_MAGIC;
    }
    FS_Write( src, len, f );
}

bool CGameArchive::Load( const char *filename )
{
    file_t file;
    ngdheader_t header;

    if ( !N_stricmp( Cvar_VariableString( "sg_savename" ), filename ) ) {
        return true; // already loaded
    }

    curfile = filename;
    Cvar_Set( "sg_savename", filename );

    if ( !VM_Call( sgvm, 0, SGAME_LOAD ) ) {
        return false;
    }

    file = FS_FOpenRead( filename );
    if ( file != FS_INVALID_HANDLE ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to open savefile '%s'.\n", filename );
        return false;
    }

    if ( FS_FileLength( file ) < sizeof(ngdheader_t) ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to load savefile '%s' because it's not big enough to have a header.\n", filename );
        return false;
    }

    FS_Read( &header, sizeof(header), file );

    if ( header.ident != IDENT ) {

    }

    if ( header.versionMajor != NOMAD_VERSION || header.versionUpdate != NOMAD_VERSION_UPDATE || header.versionPatch != NOMAD_VERSION_PATCH ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to load savefile '%s' because of an incorrect version.\n", filename );
        return false;
    }

    FS_FClose( file );

    return true;
}

bool CGameArchive::ValidateHeader( const void *header ) const
{
    const ngdheader_t *h;

    h = (const ngdheader_t *)header;

    if ( h->ident != IDENT ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to load savefile, header has incorrect identifier.\n" );
    }

    if ( h->versionMajor != NOMAD_VERSION || h->versionUpdate != NOMAD_VERSION_UPDATE || h->versionPatch != NOMAD_VERSION_PATCH ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to load savefile, header has incorrect version.\n" );
        return false;
    }
}

bool CGameArchive::LoadPartial( const char *filename, gamedata_t *gd )
{
    file_t f;
    ngdheader_t header;

    if ( FS_FileIsInBFF( filename ) ) {
        N_Error(ERR_FATAL, "Savefile '%s' was in a bff, bffs are for game resources, not save data", filename);
    }

    f = FS_FOpenRead( filename );
    if (f == FS_INVALID_HANDLE) {
        return false;
    }

    curfile = filename;

    //
    // validate the header
    //

    if ( FS_FileLength( f ) < sizeof(header) ) {
        Con_Printf( COLOR_RED "CGameArchive::Load: failed to load savefile because the file is too small to contain a header.\n" );
        return false;
    }

    FS_Read( &header, sizeof(header), f );

    if ( !ValidateHeader( &header ) ) {
        return false;
    }

    memcpy( gd, &header.gamedata, sizeof(*gd) );

    FS_FClose( f );

    return true;
}
