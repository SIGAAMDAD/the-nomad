#include "g_game.h"
#include "g_world.h"

CGameWorld *g_world;

const dirtype_t inversedirs[NUMDIRS] = {
	DIR_SOUTH,
    DIR_SOUTH_WEST,
    DIR_WEST,
    DIR_NORTH_WEST,
    DIR_NORTH,
    DIR_NORTH_EAST,
    DIR_EAST,
    DIR_SOUTH_EAST,

	DIR_NULL, // inside
};

static uint64_t CopyLump( void **dest, uint32_t lump, uint64_t size, bmf_t *header ) {
    uint64_t length, fileofs;

    length = header->map.lumps[lump].length;
    fileofs = header->map.lumps[lump].fileofs;

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

    size = FS_LoadFile( filename, &f.v );
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

	N_strncpyz( info->name, filename, sizeof( info->name ) );

    info->width = header->map.mapWidth;
    info->height = header->map.mapHeight;

    info->numCheckpoints = CopyLump( (void **)&info->checkpoints, LUMP_CHECKPOINTS, sizeof( mapcheckpoint_t ), header );
    info->numSpawns = CopyLump( (void **)&info->spawns, LUMP_SPAWNS, sizeof( mapspawn_t ), header );
    info->numTiles = CopyLump( (void **)&info->tiles, LUMP_TILES, sizeof( maptile_t ), header );
	info->numLights = CopyLump( (void **)&info->lights, LUMP_LIGHTS, sizeof( maplight_t ), header );
	info->numSecrets = CopyLump( (void **)&info->secrets, LUMP_SECRETS, sizeof( mapsecret_t ), header );
	info->numLevels = 1;

    FS_FreeFile( f.v );

    return qtrue;
}

void G_InitMapCache( void )
{
    bmf_t header;
    nhandle_t file;
    mapinfo_t *info;
    uint64_t i;
	char path[MAX_NPATH];
	char **fileList;

    Con_Printf( "Caching map files...\n" );

    memset( &gi.mapCache, 0, sizeof( gi.mapCache ) );
    fileList = FS_ListFiles( "maps/", ".bmf", &gi.mapCache.numMapFiles );

    if ( !gi.mapCache.numMapFiles ) {
        Con_Printf( "no map files to load.\n" );
        return;
    }

    Con_Printf( "Got %lu map files\n", gi.mapCache.numMapFiles );

	g_world = new ( Hunk_Alloc( sizeof( *g_world ), h_low ) ) CGameWorld();

	gi.mapCache.mapList = (char **)Hunk_Alloc( sizeof( *gi.mapCache.mapList ) * gi.mapCache.numMapFiles, h_low );
	for ( i = 0; i < gi.mapCache.numMapFiles; i++ ) {
		Com_snprintf( path, sizeof( path ) - 1, "maps/%s", fileList[i] );
		gi.mapCache.mapList[i] = (char *)Hunk_Alloc( strlen( path ) + 1, h_low );
		strcpy( gi.mapCache.mapList[i], path );
	}

	FS_FreeFileList( fileList );

    // allocate the info
//    gi.mapCache.infoList = (mapinfo_t *)Hunk_Alloc( sizeof( mapinfo_t ) * gi.mapCache.numMapFiles, h_low );
//
//    info = gi.mapCache.infoList;
//    for ( i = 0; i < gi.mapCache.numMapFiles; i++, info++ ) {
//        if ( !G_LoadLevelFile( gi.mapCache.mapList[i], info ) ) {
//            N_Error( ERR_DROP, "G_InitMapCache: failed to load map file '%s'", gi.mapCache.mapList[i] );
//        }
//    }
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
		if ( !N_stricmp( gi.mapCache.mapList[i], name ) ) {
			hMap = (nhandle_t)i + 1;
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

	Con_Printf( "\nMap List:\n" );
    for ( i = 0; i < gi.mapCache.numMapFiles; i++ ) {
		Con_Printf( "%s\n", gi.mapCache.mapList[i] );
    }
}

void G_SetMap_f( void ) {
	const char *mapname;
	nhandle_t hMap;

    mapname = Cmd_Argv( 1 );

	if ( !mapname[0] ) {
		// clear it
		Cvar_Set( "mapname", "" );
		Cbuf_ExecuteText( EXEC_NOW, "unloadworld" );

		gi.mapLoaded = qfalse;
		gi.state = GS_INACTIVE;
		gi.mapCache.currentMapLoaded = FS_INVALID_HANDLE;

		// clear all allocated map data memory
		Hunk_ClearToMark();
		return;
	}

	hMap = G_GetMapHandle( mapname );

	if ( hMap == FS_INVALID_HANDLE ) {
		Con_Printf( "Invalid map given: Aborting.\n" );
		return;
	}

    gi.mapCache.currentMapLoaded = hMap;
	gi.mapLoaded = qtrue;

	Cvar_Set( "mapname", mapname );

	Con_Printf( "Loaded map '%s'\n", mapname );
}

nhandle_t G_LoadMap( const char *name ) {
	if ( !name ) {
		N_Error( ERR_DROP, "G_LoadMap: bad parameter" );
	}

	return G_GetMapHandle( name );
}

void G_GetCheckpointData( uvec3_t xyz, uint32_t nIndex ) {
	const mapinfo_t *info;
	
	if ( !gi.mapCache.info.name[0] ) {
		N_Error( ERR_DROP, "G_GetCheckpointData: no map loaded" );
	}
	
	info = &gi.mapCache.info;
	if ( nIndex >= info->numCheckpoints ) {
		N_Error( ERR_DROP, "G_GetCheckpointData: index out of range" );
	}
	
	VectorCopy( xyz, info->checkpoints[ nIndex ].xyz );
}

void G_GetSpawnData( uvec3_t xyz, uint32_t *type, uint32_t *id, uint32_t nIndex, uint32_t *pCheckpointIndex ) {
	const mapinfo_t *info;
	
	if ( !gi.mapCache.info.name[0] ) {
		N_Error( ERR_DROP, "G_GetSpawnData: no map loaded" );
	}
	if ( !type || !id || !pCheckpointIndex ) {
		N_Error( ERR_DROP, "G_GetSpawnData: invalid parameter" );
	}
	
	info = &gi.mapCache.info;
	if ( nIndex >= info->numSpawns ) {
		N_Error( ERR_DROP, "G_GetSpawnData: index out of range" );
	}
	
	VectorCopy( xyz, info->spawns[ nIndex ].xyz );
	*type = info->spawns[ nIndex ].entitytype;
	*id = info->spawns[ nIndex ].entityid;
	*pCheckpointIndex = info->spawns[ nIndex ].checkpoint;

	Con_DPrintf( "spawn[%u] has checkpoint %u\n", nIndex, info->spawns[nIndex].checkpoint );
}

void G_GetTileData( uint32_t *pTiles, uint32_t nLevel ) {
	const mapinfo_t *info;

	if ( !pTiles ) {
		N_Error( ERR_DROP, "G_GetTileData: NULL tiles" );
	}
	if ( !gi.mapCache.info.name[0] ) {
		N_Error( ERR_DROP, "G_GetTileData: no map loaded" );
	}
	if ( nLevel >= gi.mapCache.info.numLevels ) {
		N_Error( ERR_DROP, "G_GetTileData: level index out of range" );
	}

	info = &gi.mapCache.info;

	for ( uint64_t i = 0; i < info->numTiles; i++ ) {
		pTiles[i] = info->tiles[i].flags;
	}
}

void G_SetActiveMap( nhandle_t hMap, uint32_t *nCheckpoints, uint32_t *nSpawns, uint32_t *nTiles, int32_t *pWidth, int32_t *pHeight )
{
	mapinfo_t *info;
	uint64_t start, size;
	
	if ( hMap == FS_INVALID_HANDLE || ( hMap - 1 ) >= gi.mapCache.numMapFiles ) {
		N_Error( ERR_DROP, "G_SetActiveMap: handle out of range" );
	} else if ( !nCheckpoints || !nSpawns || !nTiles ) {
		N_Error( ERR_DROP, "G_SetActiveMap: invalid parameter" );
	}
	
	info = &gi.mapCache.info;

	// set the marker before loading any map assets
    Hunk_SetMark();

	if ( !G_LoadLevelFile( gi.mapCache.mapList[ hMap - 1 ], info ) ) {
		N_Error( ERR_DROP, "G_SetActiveMap: failed to load map level file '%s'", gi.mapCache.mapList[ hMap - 1 ] );
	}
	
	*nCheckpoints = info->numCheckpoints;
	*nSpawns = info->numSpawns;
	*nTiles = info->numTiles;
	*pWidth = info->width;
	*pHeight = info->height;

	g_world->Init( &gi.mapCache.info );

	Cbuf_ExecuteText( EXEC_APPEND, va( "setmap %s\n", gi.mapCache.info.name ) );
}

CGameWorld::CGameWorld( void ) {
    memset( this, 0, sizeof(*this) );
}

CGameWorld::~CGameWorld() {
}

void CGameWorld::Init( mapinfo_t *info )
{
	m_nEntities = 0;
    m_pMapInfo = info;
    m_ActiveEnts.next =
	m_ActiveEnts.prev =
		&m_ActiveEnts;
}

void CGameWorld::LinkEntity( linkEntity_t *ent )
{
	m_hLock.WriteLock();
	m_nEntities++;

    m_ActiveEnts.prev->next = ent;
	ent->prev = m_ActiveEnts.prev;
	ent->next = &m_ActiveEnts;
	m_ActiveEnts.prev = ent;
	
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

    return m_pMapInfo->tiles[ tmp[1] * m_pMapInfo->width + tmp[0] ].sides[dir];
}

void CGameWorld::CastRay( ray_t *ray )
{
    float dx, sx;
	float dy, sy;
	float err;
	float e2;
	float angle2;
	uint32_t hitCount;
	ivec3_t end;
	vec3_t pos;
	dirtype_t rayDir;
	
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
        for ( linkEntity_t *it = m_ActiveEnts.next; it != &m_ActiveEnts; it = it->next ) {
			if ( BoundsIntersectPoint( &it->bounds, ray->origin ) ) {
				ray->entityNumber = it->entityNumber;
				return;
			}
		}
		VectorCopy( pos, ray->origin );
		Sys_SnapVector( pos );
		rayDir = inversedirs[ DirFromPoint( pos ) ];

        if ( ray->origin[0] >= ray->end[0] || ray->origin[1] >= ray->end[1] ) {
			ray->entityNumber = ENTITYNUM_INVALID;
			break;
		} else if ( m_pMapInfo->tiles[ (unsigned)pos[1] * m_pMapInfo->width + (unsigned)pos[0] ].sides[ rayDir ] ) {
			// hit a wall
			ray->entityNumber = ENTITYNUM_WALL;
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
