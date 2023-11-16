#include "g_game.h"
#include "g_archive.h"

#define NGD_MAGIC 0xff5ad1120
#define XOR_MAGIC 0xff

#pragma pack(push, 1)
typedef struct {
    // 64 bits dedicated for the version
    uint16_t versionMajor;
    uint16_t versionUpdate;
    uint32_t versionPatch;
} version_t;
#pragma pack(pop)

typedef struct {
    uint64_t magic;
    version_t v;
} ngdheader_t;

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

void GDR_ATTRIBUTE((format(printf, 2, 3))) CGameArchive::Error( const char *fmt, ... ) const
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof(msg), fmt, argptr );
    va_end( argptr );

    Con_Printf( COLOR_YELLOW "WARNING: Savefile '%s' %s, refusing to load\n", curfile, msg );
}

bool CGameArchive::Load( const char *filename )
{
    curfile = filename;

    return true;
}

bool CGameArchive::ValidateHeader( const void *header ) const
{
    const ngdheader_t *data = (const ngdheader_t *)header;

    if (data->magic != NGD_MAGIC) {
        Error("doesn't have correct numeric constant in header data");
        return false;
    }
    if (data->v.versionMajor != _NOMAD_VERSION) {
        Error("is a whole version behind this executable");
        return false;
    }

    if (data->v.versionUpdate != NOMAD_VERSION_UPDATE || data->v.versionPatch != NOMAD_VERSION_PATCH) {
        Con_Printf( COLOR_YELLOW "WARNING: Savefile '%s' version is less than this executable's version\n", curfile );
    }

    return true;
}

bool CGameArchive::LoadPartial( const char *filename, gamedata_t *gd )
{
    file_t f;
    ngdheader_t header;

    if (FS_FileIsInBFF( filename )) {
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

    if (FS_FileLength( f ) < sizeof(header)) {
        Error("isn't big enough to contain header data");
        return false;
    }

    FS_Read( &header, sizeof(header), f );

    if (!ValidateHeader( &header )) {
        return false;
    }

    FS_Read( gd, sizeof(*gd), f );

    FS_FClose( f );

    return true;
}
