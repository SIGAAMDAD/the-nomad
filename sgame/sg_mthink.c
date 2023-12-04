#include "sg_local.h"

//======================================================

mobj_t *SG_SpawnMob( mobtype_t type )
{
	mobj_t *m;
	sgentity_t *e;
	
	if (type >= NUMMOBS) {
		SG_Error("SG_SpawnMob: bad mob index");
	}
	
	e = SG_AllocEntity( ET_MOB );
	if (!e) {
		SG_Error("SG_SpawnMob: failed to allocate entity");
    }

    m = e->entPtr;
	memcpy(m, &mobinfo, sizeof(*m));

    switch (type) {
    case MT_CHAINSAW:
        e->stateOffset = 0;
        break;
    case MT_SHOTTY:
        e->stateOffset = ST_SHOTTY_IDLE;
        break;
    case MT_GRUNT:
        e->stateOffset = ST_GRUNT_IDLE;
        break;
    case MT_HULK:
        break;
    };
    e->state = &stateinfo[ ST_IDLE + e->stateOffset ];
    e->ticker = e->state->ticcount;
	
	return m;
}

void SG_KillMob( mobj_t *m )
{
	SG_FreeEntity( m->ent );
}

void SG_SpawnMobOnMap( mobtype_t id, float x, float y, float elevation )
{
	mobj_t *m;
	
	m = SG_SpawnMob( id );
	
	m->ent->origin[0] = x;
	m->ent->origin[1] = y;
	m->ent->origin[2] = elevation;
}
