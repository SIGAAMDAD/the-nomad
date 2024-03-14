#ifndef __G_WORLD__
#define __G_WORLD__

#pragma once

#include "../engine/n_threads.h"

class CGameWorld
{
public:
    CGameWorld( void );
    ~CGameWorld();

    void Init( mapinfo_t *info, float *soundBits, linkEntity_t *activeEnts );
    void LinkEntity( linkEntity_t *ent );
    void UnlinkEntity( linkEntity_t *ent );
    void CastRay( ray_t *ray );
    void SoundRecursive( int32_t width, int32_t height, float volume, const vec3_t origin );
    qboolean CheckWallHit( const vec3_t origin, dirtype_t dir );

    inline uint32_t GetWidth( void ) const {
        return m_pMapInfo->width;
    }
    inline uint32_t GetHeight( void ) const {
        return m_pMapInfo->height;
    }
private:
    linkEntity_t *m_pActiveEnts;
    linkEntity_t *m_pEndEnt;
    mapinfo_t *m_pMapInfo;
    float *m_pSoundBits;

    CThreadRWMutex m_hLock;
};

void G_GetCheckpointData( uvec3_t xyz, uint32_t nIndex );
void G_GetSpawnData( uvec3_t xyz, uint32_t *type, uint32_t *id, uint32_t nIndex );

extern CGameWorld g_world;

#endif