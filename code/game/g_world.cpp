#include "g_game.h"
#include "g_world.h"

CGameWorld *g_world;

static uint64_t CopyLump( void **dest, uint32_t lump, uint64_t size, mapheader_t *header ) {
    uint64_t length, fileofs;

    length = header->lumps[lump].length;
    fileofs = header->lumps[lump].fileofs;

    if ( length % size ) {
        N_Error( ERR_DROP, "CopyLump: funny lump size" );
    }
    *dest = Hunk_Alloc( length, h_low );
    memcpy( *dest, (byte *)header + fileofs, length );

    return length / size;
}

static qboolean G_LoadLevelFile( const char *filename, mapinfo_t *info )
{
    union {
        char *b;
        void *v;
    } f;
    bmf_t *header;
    uint64_t size;
    char realpath[MAX_NPATH];

    Com_snprintf( realpath, sizeof(realpath), "maps/%s", filename );

    size = FS_LoadFile( realpath, &f.v );
    if ( !size || !f.v ) {
        Con_Printf( COLOR_YELLOW "WARNING: failed to load map file '%s'\n", filename );
        return qfalse;
    }

    header = (bmf_t *)f.b;
    if ( size < sizeof(*header) ) {
        Con_Printf( COLOR_YELLOW "WARNING: map file '%s' isn't big enough to be a map file\n", filename );
        return qfalse;
    }
    
    if ( header->ident != LEVEL_IDENT ) {
        Con_Printf( COLOR_YELLOW "WARNING: map file '%s' has bad identifier\n", filename );
        return qfalse;
    }
    if ( header->version != LEVEL_VERSION ) {
        Con_Printf( COLOR_YELLOW "WARNING: bad map version (%i (it) != %i (this)) in file '%s'\n", header->version, LEVEL_VERSION, filename );
        return qfalse;
    }
    
//    N_strncpyz( info->name, COM_SkipPath( const_cast<char *>( filename ) ), sizeof( info->name ) );

    info->width = header->map.mapWidth;
    info->height = header->map.mapHeight;

    info->numCheckpoints = CopyLump( (void **)&info->checkpoints, LUMP_CHECKPOINTS, sizeof( mapcheckpoint_t ), &header->map );
    info->numSpawns = CopyLump( (void **)&info->spawns, LUMP_SPAWNS, sizeof( mapspawn_t ), &header->map );
    info->numTiles = CopyLump( (void **)&info->tiles, LUMP_TILES, sizeof( maptile_t ), &header->map );

    FS_FreeFile( f.v );

    return qtrue;
}

void G_InitMapCache( void )
{
    bmf_t header;
    nhandle_t file;
    mapinfo_t *info;
    uint64_t i;

    Con_Printf( "Caching map files...\n" );

    memset( &gi.mapCache, 0, sizeof( gi.mapCache ) );
    gi.mapCache.mapList = FS_ListFiles( "maps/", ".bmf", &gi.mapCache.numMapFiles );

    if ( !gi.mapCache.numMapFiles ) {
        Con_Printf( "no map files to load.\n" );
        return;
    }

    Con_Printf( "Got %lu map files\n", gi.mapCache.numMapFiles );

    // allocate the info
    gi.mapCache.infoList = (mapinfo_t *)Hunk_Alloc( sizeof( mapinfo_t ) * gi.mapCache.numMapFiles, h_low );

    info = gi.mapCache.infoList;
    for ( i = 0; i < gi.mapCache.numMapFiles; i++, info++ ) {
        if ( !G_LoadLevelFile( gi.mapCache.mapList[i], info ) ) {
            N_Error( ERR_DROP, "G_InitMapCache: failed to load map file '%s'", gi.mapCache.mapList[i] );
        }
    }
}

static nhandle_t G_GetMapHandle( const char *name )
{
	nhandle_t hMap;
	uint64_t i;

	if ( !name ) {
		N_Error( ERR_DROP, "G_LoadMap: bad parameter" );
	}

	hMap = FS_INVALID_HANDLE;
	for ( i = 0; i < gi.mapCache.numMapFiles; i++ ) {
		if ( !N_stricmp( gi.mapCache.infoList[i].name, name ) ) {
			hMap = (nhandle_t)i;
			break;
		}
	}
	if ( hMap == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: couldn't find map file '%s'\n", name );
	}

	return hMap;
}

void G_MapInfo_f( void ) {
	uint64_t i;

    Con_Printf( "---------- Map Info ----------\n" );
    for ( i = 0; i < gi.mapCache.numMapFiles; i++ ) {
        Con_Printf( "[Map %lu] >\n", i );
        Con_Printf( "Name: %s\n", gi.mapCache.infoList[i].name );
        Con_Printf( "Checkpoint Count: %u\n", gi.mapCache.infoList[i].numCheckpoints );
        Con_Printf( "Spawn Count: %u\n", gi.mapCache.infoList[i].numSpawns );
        Con_Printf( "Map Width: %u\n", gi.mapCache.infoList[i].width );
        Con_Printf( "Map Height: %u\n", gi.mapCache.infoList[i].height );
    }
}

void G_SetMap_f( void ) {
	const char *mapname;
	nhandle_t hMap;

	if ( Cmd_Argc() < 2 ) {
		Con_Printf( "usage: setmap <name>\n" );
		return;
    }

    mapname = Cmd_Argv( 1 );

    if ( !N_stricmp( mapname, "none" ) ) {
        gi.mapCache.currentMapLoaded = FS_INVALID_HANDLE;
        return;
    }

	hMap = G_GetMapHandle( mapname );

	if ( hMap == FS_INVALID_HANDLE ) {
		Con_Printf( "Invalid map given: Aborting.\n" );
		return;
	}

    gi.mapCache.currentMapLoaded = hMap;
}

nhandle_t G_LoadMap( const char *name ) {
	if ( !name ) {
		N_Error( ERR_DROP, "G_LoadMap: bad parameter" );
	}

	return G_GetMapHandle( name );
}

void G_GetCheckpointData( uvec3_t xyz, uint32_t nIndex ) {
	const mapinfo_t *info;
	
	if ( gi.mapCache.currentMapLoaded == FS_INVALID_HANDLE ) {
		N_Error( ERR_DROP, "G_GetCheckpointData: no map loaded" );
	}
	
	info = &gi.mapCache.infoList[ gi.mapCache.currentMapLoaded ];
	if ( nIndex >= info->numCheckpoints ) {
		N_Error( ERR_DROP, "G_GetCheckpointData: index out of range" );
	}
	
	VectorCopy( xyz, info->checkpoints[ nIndex ].xyz );
}

void G_GetSpawnData( uvec3_t xyz, uint32_t *type, uint32_t *id, uint32_t nIndex ) {
	const mapinfo_t *info;
	
	if ( gi.mapCache.currentMapLoaded == FS_INVALID_HANDLE ) {
		N_Error( ERR_DROP, "G_GetSpawnData: no map loaded" );
	}
	
	info = &gi.mapCache.infoList[ gi.mapCache.currentMapLoaded ];
	if ( nIndex >= info->numSpawns ) {
		N_Error( ERR_DROP, "G_GetSpawnData: index out of range" );
	}
	
	VectorCopy( xyz, info->spawns[ nIndex ].xyz );
	*type = info->spawns[ nIndex ].entitytype;
	*id = info->spawns[ nIndex ].entityid;
}

void G_GetTileData( uint32_t *pTiles ) {
	const mapinfo_t *info;

	if ( !pTiles ) {
		N_Error( ERR_DROP, "G_GetTileData: NULL tiles" );
	}
	if ( gi.mapCache.currentMapLoaded == FS_INVALID_HANDLE ) {
		N_Error( ERR_DROP, "G_GetTileData: no map loaded" );
	}

	info = &gi.mapCache.infoList[ gi.mapCache.currentMapLoaded ];
	memcpy( pTiles, info->tiles, sizeof( *info->tiles ) * info->numTiles );
}

void G_SetActiveMap( nhandle_t hMap, uint32_t *nCheckpoints, uint32_t *nSpawns, uint32_t *nTiles,
	float *soundBits, linkEntity_t *activeEnts )
{
	const mapinfo_t *info;
	
	if ( hMap == FS_INVALID_HANDLE || hMap >= gi.mapCache.numMapFiles ) {
		N_Error( ERR_DROP, "G_SetActiveMap: handle out of range" );
	} else if ( !info || !soundBits || !activeEnts ) {
		N_Error( ERR_DROP, "G_SetActiveMap: invalid parameter" );
	}
	
	info = &gi.mapCache.infoList[ hMap ];
	*nCheckpoints = info->numCheckpoints;
	*nSpawns = info->numSpawns;
	*nTiles = info->numTiles;
	
	g_world->Init( &gi.mapCache.infoList[ hMap ], soundBits, activeEnts );

	Cbuf_ExecuteText( EXEC_APPEND, va( "setmap %s 0\n", gi.mapCache.infoList[ hMap ].name ) );
}

CGameWorld::CGameWorld( void ) {
    memset( this, 0, sizeof(*this) );
}

CGameWorld::~CGameWorld() {
}

void CGameWorld::Init( mapinfo_t *info, float *soundBits, linkEntity_t *activeEnts )
{
    if ( !soundBits ) {
        N_Error( ERR_DROP, "CGameWorld::Init: invalid soundBits data provided!" );
    }
    if ( !activeEnts ) {
        N_Error( ERR_DROP, "CGameWorld::Init: invalid activeEnts data provided!" );
    }

    m_pMapInfo = info;
    m_pSoundBits = soundBits;
    m_pActiveEnts = activeEnts;
	m_nEntities = 0;

    m_pEndEnt = m_pActiveEnts;
    m_pActiveEnts->next = m_pActiveEnts->prev = m_pActiveEnts;
}

void CGameWorld::LinkEntity( linkEntity_t *ent )
{
	m_hLock.WriteLock();
    ent->prev = m_pEndEnt;
    ent->next = m_pActiveEnts;

	m_nEntities++;

    m_pEndEnt->next = ent;
    m_pEndEnt = ent;
	m_hLock.WriteUnlock();
}

void CGameWorld::UnlinkEntity( linkEntity_t *ent )
{
    ent->prev->next = ent->next;
    ent->next->prev = ent->prev;
}

qboolean CGameWorld::CheckWallHit( const vec3_t origin, dirtype_t dir )
{
    vec3_t p;
	ivec3_t tmp;
    VectorCopy( p, origin );
	Sys_SnapVector( p );
	VectorCopy( tmp, p );

    return m_pMapInfo->tiles[ tmp[1] * tmp[0] + m_pMapInfo->width ].sides[dir];
}

void CGameWorld::CastRay( ray_t *ray )
{
    float dx, sx;
	float dy, sy;
	float err;
	float e2;
	float angle2;
	uint32_t hitCount;
	ivec3_t pos, end;
	
	// calculate the endpoint
	angle2 = DEG2RAD( ray->angle );
	ray->end[0] = ray->start[0] + ray->length * cos( angle2 );
	ray->end[1] = ray->start[1] + ray->length * sin( angle2 );
	ray->end[2] = ray->start[2]; // just elevation
	
	dx = abs( ray->end[0] - ray->start[0] );
	dy = abs( ray->end[1] - ray->start[1] );
	sx = ray->start[0] < ray->end[0] ? 1 : -1;
	sy = ray->start[1] < ray->end[1] ? 1 : -1;
	err = ( dx > dy ? dx : -dy ) / 2;
	VectorCopy( ray->origin, ray->start );
	
	hitCount = 0;
	for ( ;; ) {
        for ( linkEntity_t *it = m_pActiveEnts->next; ; it = it->next ) {
			if ( BoundsIntersectPoint( &it->bounds, ray->origin ) ) {
				ray->entityNumber = it->entityNumber;
				break;
			}
        }

        if ( ray->origin[0] == ray->end[0] && ray->origin[1] == ray->end[1] ) {
			ray->entityNumber = m_nEntities;
			break;
		}
		
		e2 = err;
		if ( e2 > -dx ) {
			err -= dy;
			ray->origin[0] += sx;
		}
		if ( e2 < dy ) {
			err += dx;
			ray->origin[1] += sy;
		}
	}
}

void CGameWorld::SoundRecursive( int32_t width, int32_t height, float volume, const vec3_t origin )
{
	int32_t y, x;
	ivec3_t start, end;
	vec3_t pos;
	const float rangeX = ceil( (float)width / 2.0f );
	const float rangeY = ceil( (float)height / 2.0f );
	
	VectorCopy( pos, origin );
	Sys_SnapVector( pos );
	
	start[0] = pos[0] - rangeX;
	start[1] = pos[1] - rangeY;
	start[2] = pos[2];
	
	end[0] = pos[0] + rangeX;
	end[1] = pos[1] + rangeY;
	end[2] = pos[2];
	
	
}