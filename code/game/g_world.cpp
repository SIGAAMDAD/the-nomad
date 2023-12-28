#include "g_game.h"
#include "g_world.h"

CGameWorld g_world;

CGameWorld::CGameWorld( void ) {
    memset( this, 0, sizeof(*this) );
}

CGameWorld::~CGameWorld() {
}

void CGameWorld::Init( mapinfoReal_t *info, int32_t *soundBits, linkEntity_t *activeEnts )
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

    m_pEndEnt = m_pActiveEnts;
    m_pActiveEnts->next = m_pActiveEnts->prev = m_pActiveEnts;
}

void CGameWorld::LinkEntity( linkEntity_t *ent )
{
    ent->prev = m_pEndEnt;
    ent->next = m_pActiveEnts;

    m_pEndEnt->next = ent;
    m_pEndEnt = ent;
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

    return m_pMapInfo->tiles[ tmp[1] * tmp[0] + m_pMapInfo->info.width ].sides[dir];
}

void CGameWorld::CastRay( ray_t *ray )
{
    int32_t dx, sx;
	int32_t dy, sy;
	int32_t err;
	int32_t e2;
	float angle2;
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
	
	for ( ;; ) {
        for ( linkEntity_t *it = m_pActiveEnts->next;; it = it->next ) {
        }

        if ( ray->origin[0] == ray->end[0] && ray->origin[1] == ray->end[1] ) {
			break;
		}
		
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			ray->origin[0] += sx;
		}
		if (e2 < dy) {
			err += dx;
			ray->origin[1] += sy;
		}
	}
}

void CGameWorld::SoundRecursive( int32_t width, int32_t height, float volume, const vec3_t origin )
{

}
