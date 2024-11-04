#ifndef __G_WORLD__
#define __G_WORLD__

#pragma once

#include "../engine/n_threads.h"

class CGameWorld
{
public:
    CGameWorld( void );
    ~CGameWorld();

    void Init( mapinfo_t *info );
    void LinkEntity( linkEntity_t *ent );
    void UnlinkEntity( linkEntity_t *ent );
    void CastRay( ray_t *ray );
    qboolean CheckWallHit( const vec3_t origin, dirtype_t dir );

    inline uint32_t GetWidth( void ) const {
        return m_pMapInfo->width;
    }
    inline uint32_t GetHeight( void ) const {
        return m_pMapInfo->height;
    }
    inline uint32_t NumEntities( void ) const {
        return m_nEntities;
    }
    inline linkEntity_t *GetEntities( void ) {
        return &m_ActiveEnts;
    }
private:
    linkEntity_t m_ActiveEnts;
    mapinfo_t *m_pMapInfo;
    uint32_t m_nEntities;
};

extern const uint64_t tileside_flags[ NUMDIRS ];
extern const uint64_t inversedirs_flags[ NUMDIRS ];

#define IsWall( dir, flags ) (qboolean)( ( flags ) & tileside_flags[ dir ] )
#define IsDoubleSidedWall( dir, flags ) (qboolean)( ( flags ) & inversedirs_flags[ dir ] )

#define ENTITYNUM_INVALID (unsigned)( ~0 )
#define ENTITYNUM_WALL ENTITYNUM_INVALID - 1
#define MAX_ENTITIES 4096

void G_GetTileData( uint32_t *pTiles, uint32_t nLevel );
void G_GetCheckpointData( uvec3_t xyz, uvec2_t areaLock, uint32_t nIndex );
void G_GetSpawnData( uvec3_t xyz, uint32_t *type, uint32_t *id, uint32_t nIndex, uint32_t *pCheckpointIndex );
void G_GetSecretData( uint32_t *pCheckpointIndex, uint32_t nIndex );
void G_GetMapData( maptile_t **tiles, uint32_t *numTiles );

extern const dirtype_t inversedirs[NUMDIRS];
extern CGameWorld *g_world;

#endif