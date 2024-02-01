#ifndef __G_WORLD__
#define __G_WORLD__

#pragma once

#include "../engine/n_threads.h"

class CGameWorld
{
public:
    CGameWorld( void );
    ~CGameWorld();

    void Init( mapinfoReal_t *info, int32_t *soundBits, linkEntity_t *activeEnts );
    void LinkEntity( linkEntity_t *ent );
    void UnlinkEntity( linkEntity_t *ent );
    void CastRay( ray_t *ray );
    void SoundRecursive( int32_t width, int32_t height, float volume, const vec3_t origin );
    qboolean CheckWallHit( const vec3_t origin, dirtype_t dir );

    inline uint32_t GetWidth( void ) const {
        return m_pMapInfo->info.width;
    }
    inline uint32_t GetHeight( void ) const {
        return m_pMapInfo->info.height;
    }
private:
    linkEntity_t *m_pActiveEnts;
    linkEntity_t *m_pEndEnt;
    mapinfoReal_t *m_pMapInfo;
    int32_t *m_pSoundBits;

    CThreadRWMutex m_hLock;
};

extern CGameWorld g_world;

#endif