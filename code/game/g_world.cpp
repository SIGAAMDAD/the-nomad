#include "g_game.h"
#include "g_world.h"
#include "../sound/snd_local.h"

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

const uint64_t tileside_flags[ NUMDIRS ] = {
	TILESIDE_NORTH,
	TILESIDE_NORTH_EAST,
	TILESIDE_EAST,
	TILESIDE_SOUTH_EAST,
	TILESIDE_SOUTH,
	TILESIDE_SOUTH_WEST,
	TILESIDE_WEST,
	TILESIDE_NORTH_WEST,

	TILESIDE_INSIDE
};

const uint64_t inversedirs_flags[ NUMDIRS ] = {
	TILESIDE_SOUTH,
	TILESIDE_SOUTH_WEST,
	TILESIDE_WEST,
	TILESIDE_NORTH_WEST,
	TILESIDE_NORTH,
	TILESIDE_NORTH_EAST,
	TILESIDE_EAST,
	TILESIDE_SOUTH_EAST,

	TILESIDE_INSIDE
};

static uint64_t CopyLump( void **dest, uint32_t lump, uint64_t size, bmf_t *header ) {
	uint64_t length, fileofs;

	length = header->map.lumps[lump].length;
	fileofs = header->map.lumps[lump].fileofs;

	if ( length % size ) {
		N_Error( ERR_DROP, "CopyLump: funny lump size" );
	}
	*dest = Hunk_Alloc( length, h_high );
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
	uint64_t i;
	ivec2_t *coords, *p;
	int x, y;
	const byte nowall[NUMDIRS] = { 0 };

	size = FS_LoadFile( filename, &f.v );
	if ( !size || !f.v ) {
		Con_Printf( COLOR_YELLOW "WARNING: failed to load map file '%s'\n", filename );
		return qfalse;
	}

	header = (bmf_t *)f.b;
	if ( size < sizeof( *header ) ) {
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

	info->numTiles = CopyLump( (void **)&info->tiles, LUMP_TILES, sizeof( maptile_t ), header );
	info->numCheckpoints = CopyLump( (void **)&info->checkpoints, LUMP_CHECKPOINTS, sizeof( mapcheckpoint_t ), header );
	info->numSpawns = CopyLump( (void **)&info->spawns, LUMP_SPAWNS, sizeof( mapspawn_t ), header );
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

	gi.mapCache.mapList = (char **)Hunk_Alloc( sizeof( *gi.mapCache.mapList ) * gi.mapCache.numMapFiles, h_low );
	for ( i = 0; i < gi.mapCache.numMapFiles; i++ ) {
		Com_snprintf( path, sizeof( path ) - 1, "maps/%s", fileList[i] );
		gi.mapCache.mapList[i] = (char *)Hunk_Alloc( strlen( path ) + 1, h_low );
		strcpy( gi.mapCache.mapList[i], path );
	}

	FS_FreeFileList( fileList );
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
		Con_Printf( "- %s\n", gi.mapCache.mapList[i] );
	}
}

static void ListActiveEntities_f( void )
{
	const linkEntity_t *ent;

	Con_Printf( "\n%u Active Entities:\n", g_world->NumEntities() );
	for ( ent = g_world->GetEntities()->next; ent != g_world->GetEntities(); ent = ent->next ) {
		Con_Printf( "%-4u: %i (id) %i (type)\n", ent->entityNumber, ent->id, ent->type );
	}
}

void G_SetMap_f( void ) {
	const char *mapname;
	nhandle_t hMap;

	mapname = Cmd_Argv( 1 );

	if ( !mapname[0] ) {
		// clear it
		Cvar_Set( "mapname", "" );

		gi.mapLoaded = qfalse;
		gi.state = GS_INACTIVE;
		gi.mapCache.currentMapLoaded = FS_INVALID_HANDLE;

		g_world = NULL;

		Cmd_RemoveCommand( "list_active_ents" );

		Hunk_ClearToMark();

		Cbuf_ExecuteText( EXEC_NOW, "vid_restart keep_context\n" );
		
		const char *nextmap = Cvar_VariableString( "nextmap" );
		
		// we've triggered a map transition
		if ( nextmap[0] ) {
			Con_Printf( "Map transition triggered...\n" );
			Cbuf_ExecuteText( EXEC_NOW, va( "setmap %s\n", nextmap ) );
		}
		return;
	}

	hMap = G_GetMapHandle( mapname );

	if ( hMap == FS_INVALID_HANDLE ) {
		Con_Printf( "Invalid map given: Aborting.\n" );
		return;
	}

	Cmd_AddCommand( "list_active_ents", ListActiveEntities_f );

	gi.mapCache.currentMapLoaded = hMap;
	gi.mapLoaded = qtrue;
	gi.state = GS_LEVEL;

	Cvar_SetIntegerValue( "g_paused", 0 );
	Cvar_SetIntegerValue( "g_levelIndex", hMap );

	Cvar_Set( "mapname", gi.mapCache.info.name );

	Con_Printf( "Loaded map '%s'\n", mapname );
}

nhandle_t G_LoadMap( const char *name ) {
	if ( !name ) {
		N_Error( ERR_DROP, "G_LoadMap: bad parameter" );
	}

	return G_GetMapHandle( name );
}

void G_GetCheckpointData( uvec3_t xyz, uvec2_t areaLock, uint32_t nIndex ) {
	const mapinfo_t *info;
	
	if ( !gi.mapCache.info.name[0] ) {
		N_Error( ERR_DROP, "G_GetCheckpointData: no map loaded" );
	}
	
	info = &gi.mapCache.info;
	if ( nIndex >= info->numCheckpoints ) {
		N_Error( ERR_DROP, "G_GetCheckpointData: index out of range" );
	}
	
	VectorCopy2( xyz, info->checkpoints[ nIndex ].xyz );
	VectorCopy2( areaLock, info->checkpoints[ nIndex ].lockArea );
}

void G_GetSpawnData( uvec3_t xyz, uint32_t *type, uint32_t *id, uint32_t nIndex, uint32_t *pCheckpointIndex ) {
	const mapinfo_t *info;
	
	if ( !gi.mapCache.info.name[0] ) {
		N_Error( ERR_DROP, "G_GetSpawnData: no map loaded" );
	}
	if ( !type || !id || !pCheckpointIndex ) {
		N_Error( ERR_DROP, "G_GetSpawnData: invalid parameter" );
	}

	Hunk_SetMark();
	
	info = &gi.mapCache.info;
	if ( nIndex >= info->numSpawns ) {
		N_Error( ERR_DROP, "G_GetSpawnData: index out of range" );
	}

	VectorCopy2( xyz, info->spawns[ nIndex ].xyz );
	*type = info->spawns[ nIndex ].entitytype;
	*id = info->spawns[ nIndex ].entityid;
	*pCheckpointIndex = info->spawns[ nIndex ].checkpoint;

	Con_DPrintf( "spawn[%u] has checkpoint %u (%u:%u)\n", nIndex, info->spawns[nIndex].checkpoint,
		info->spawns[nIndex].entitytype, info->spawns[nIndex].entityid );
}

void G_GetTileData( uint64_t *pTiles, uint32_t nLevel ) {
	const mapinfo_t *info;
	uint64_t i;

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
	
	for ( i = 0; i < info->numTiles; i++ ) {
		pTiles[i] = info->tiles[i].flags;
	}
}

void G_GetMapData( maptile_t **tiles, uint32_t *numTiles )
{
	*tiles = gi.mapCache.info.tiles;
	*numTiles = gi.mapCache.info.numTiles;
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
	if ( !G_LoadLevelFile( gi.mapCache.mapList[ hMap - 1 ], info ) ) {
		N_Error( ERR_DROP, "G_SetActiveMap: failed to load map level file '%s'", gi.mapCache.mapList[ hMap - 1 ] );
	}
	
	*nCheckpoints = info->numCheckpoints;
	*nSpawns = info->numSpawns;
	*nTiles = info->numTiles;
	*pWidth = info->width;
	*pHeight = info->height;

	static CGameWorld gameWorld;
	g_world = &gameWorld;
	g_world->Init( &gi.mapCache.info );
	Key_SetCatcher( Key_GetCatcher() | KEYCATCH_SGAME );

	static CSoundWorld soundWorld;
	s_SoundWorld = &soundWorld;
	s_SoundWorld->Init();

	Cbuf_ExecuteText( EXEC_APPEND, va( "setmap %s\n", gi.mapCache.info.name ) );
}

CGameWorld::CGameWorld( void ) {
	memset( this, 0, sizeof( *this ) );
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
	
	m_ActiveEnts.entityNumber = MAX_ENTITIES;
}

void CGameWorld::LinkEntity( linkEntity_t *ent )
{
	m_ActiveEnts.prev->next = ent;
	ent->prev = m_ActiveEnts.prev;
	ent->next = &m_ActiveEnts;
	m_ActiveEnts.prev = ent;

	m_nEntities++;
	
	Con_Printf( "Allocated link entity %u (%u type, %u id)\n", ent->entityNumber, ent->type, ent->id );
}

void CGameWorld::UnlinkEntity( linkEntity_t *ent )
{
	ent->prev->next = ent->next;
	ent->next->prev = ent->prev;

	Con_Printf( "Removing link entity %u\n", ent->entityNumber );

	m_nEntities--;
}

qboolean CGameWorld::CheckWallHit( const vec3_t origin, dirtype_t dir )
{
	vec3_t p;
	ivec3_t tmp;
	VectorCopy( p, origin );

	/*
	switch ( dir ) {
	case DIR_NORTH:
		p[1] -= 1.0f;
		break;
	case DIR_SOUTH:
		p[1] += 1.0f;
		break;
	case DIR_EAST:
		p[0] += 1.0f;
		break;
	case DIR_NORTH_EAST:
		p[0] += 1.0f;
		p[1] -= 1.0f;
		break;
	case DIR_NORTH_WEST:
		p[1] -= 1.0f;
		break;
	case DIR_SOUTH_EAST:
		p[0] += 1.0f;
		p[1] += 1.0f;
		break;
	case DIR_SOUTH_WEST:
		p[1] += 1.0f;
		break;
	};
	*/

	Sys_SnapVector( p );
	VectorCopy( tmp, p );

	tmp[0] = Com_Clamp( 0, m_pMapInfo->width, tmp[0] );
	tmp[1] = Com_Clamp( 0, m_pMapInfo->height, tmp[1] );

	if ( m_pMapInfo->tiles[ tmp[1] * m_pMapInfo->width + tmp[0] ].flags & TILESIDE_INSIDE 
		|| m_pMapInfo->tiles[ tmp[1] * m_pMapInfo->width + tmp[0] ].flags & inversedirs_flags[ dir ] )
	{
		return qtrue;
	}

	return qfalse;
}

void CGameWorld::CastRay( ray_t *ray )
{
	PROFILE_FUNCTION();

	float dx, sx;
	float dy, sy;
	float err;
	float e2;
	vec3_t end;
	vec3_t p;
	dirtype_t rayDir;
	uint64_t i;
	float angle2;
	linkEntity_t *it;
	vec3_t tmp;
	
	// calculate the endpoint
	ray->start[0] /= 10.0f;
	ray->start[1] /= 10.0f;
	ray->start[2] /= 10.0f;

	ray->end[0] = ray->start[0] + ( ray->length * cos( ray->angle ) );
	ray->end[1] = ray->start[1] + ( ray->length * sin( ray->angle ) );
	ray->end[2] = ray->start[2] * sin( ray->angle );

	dx = abs( ray->end[0] - ray->start[0] );
	dy = abs( ray->end[1] - ray->start[1] );
	sx = ray->end[0] > ray->start[0] ? ray->speed : -ray->speed;
	sy = ray->end[1] > ray->start[1] ? ray->speed : -ray->speed;
	err = ( dx > dy ? dx : -dy ) / 2.0f;
	VectorCopy( ray->origin, ray->start );

	angle2 = RAD2DEG( ray->angle );
	if ( angle2 < 0.0f ) {
		angle2 += 360.0f;
	}

	for ( ;; ) {
		for ( it = m_ActiveEnts.next; it != &m_ActiveEnts; it = it->next ) {
			if ( it->entityNumber != ray->ownerNumber && it->entityNumber != ray->ownerNumber2
				&& BoundsIntersectPoint( &it->bounds, ray->origin ) )
			{
				switch ( it->type ) {
				case ET_WALL:
				case ET_BOT:
				case ET_ITEM:
				case ET_WEAPON:
					break;
				case ET_PLAYR:
					if ( Cvar_VariableInteger( "sgame_NoClip" ) ) {
						continue;
					}
				case ET_MOB:
					ray->entityNumber = it->entityNumber;
					return;
				};
			}
		}

		if ( ( sy == -ray->speed && ray->origin[1] <= ray->end[1] ) || ( sy == ray->speed && ray->origin[1] >= ray->end[1] )
			|| ( sx == -ray->speed && ray->origin[0] <= ray->end[0] ) || ( sx == ray->speed && ray->origin[0] >= ray->end[0] ) )
		{
			ray->entityNumber = ENTITYNUM_INVALID;
			return;
		}

		rayDir = inversedirs[ Angle2Dir( angle2 ) ];

		if ( CheckWallHit( ray->origin, rayDir ) ) {
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
