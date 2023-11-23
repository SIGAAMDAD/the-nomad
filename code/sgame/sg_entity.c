#include "sg_local.h"

sgentity_t sg_entList[MAXENTITIES];
// doubly-linked list of allocated entities
sgentity_t sg_activeEnts;
// single-linked list of unused entities
sgentity_t *sg_freeEnts;

// Use a heuristic approach to detect infinite state cycles: Count the number
// of times the loop in Ent_SetState() executes and exit with an error once
// an arbitrary very large limit is reached.

#define ENTITY_CYCLE_LIMIT 100000

qboolean Ent_SetState(sgentity_t *self, statenum_t state)
{
	state_t *st;
	uint32_t counter;
	
	do {
		if (state == ST_NULL) {
			self->state = &stateinfo[ ST_NULL ];
			SG_FreeEntity(self);
			return qfalse;
		}
		
		st = &stateinfo[state];
		self->state = st;
		self->ticker = st->ticcount;
		self->sprite = st->sprite;
		
		state = st->next;
		if (counter++ > ENTITY_CYCLE_LIMIT) {
			G_Error("Ent_SetState: infinite state cycle detected!");
		}
	} while (!self->ticker);
	
	return qtrue;
}

void SG_BuildBounds( bbox_t *bounds, const vec3_t origin, float w, float h )
{
	w /= 2;
	h /= 2;

	bounds->mins[0] = origin[0] - h;
	bounds->mins[1] = origin[1] - w;
	bounds->mins[2] = origin[2];

	bounds->maxs[0] = origin[0] + h;
	bounds->maxs[1] = origin[1] + w;
	bounds->maxs[2] = origin[2];
}

void SG_InitEntities(void)
{
	uint32_t i;
	
	memset( sg_entList, 0, sizeof(sg_entList) );
	sg_activeEnts.next = &sg_activeEnts;
	sg_activeEnts.prev = &sg_activeEnts;
	sg_freeEnts = sg_entList;
	
	for (i = 0; i < MAXENTITIES; i++) {
		sg_entList[i].next = &sg_entList[i+1];
	}
}

sgentity_t *SG_FindEntityAt(const vec3_t pos)
{
	sgentity_t *e;
	
	for (e = sg_activeEnts.next; e; e = e->next) {
		if (BoundsIntersectPoint( e->bounds.mins, e->bounds.maxs, pos )) {
			return e;
		}
	}
	return NULL;
}

void SG_FreeEntity(sgentity_t *e)
{
	if (!e->prev) {
		G_Error("SG_FreeEntity: not active");
	}
	
	// remove from the doubly linked active list
	e->prev->next = e->next;
	e->next->prev = e->prev;

	switch (e->type) {
	case ET_MOB:
		memset( e->entPtr, 0, sizeof(mobj_t) );
		sg.numMobs--;
		break;
	case ET_ITEM:
		memset( e->entPtr, 0, sizeof(item_t) );
		sg.numItems--;
		break;
	case ET_WEAPON:
		memset( e->entPtr, 0, sizeof(weapon_t) );
		sg.numWeapons--;
		break;
	};
	
	e->next = sg_freeEnts;
	sg_freeEnts = e;
}

sgentity_t *SG_AllocEntity(entitytype_t type)
{
	sgentity_t *e;
	
	if (!sg_freeEnts) {
		// no free entities, so free the one at the nd of the chain
		// remove the oldest active entity
		SG_FreeEntity(sg_activeEnts.prev);
	}
	
	e = sg_freeEnts;
	sg_freeEnts = sg_freeEnts->next;
	
	memset(e, 0, sizeof(*e));

	switch (type) {
	case ET_PLAYR:
		e->entPtr = &sg.playr;
		sg.playr.ent = e;
		break;
	case ET_MOB:
		e->entPtr = &sg.mobs[sg.numMobs];
		sg.mobs[sg.numMobs].ent = e;
		sg.numMobs++;
		break;
	case ET_ITEM:
		e->entPtr = &sg.items[sg.numItems];
		sg.numItems++;
		break;
	case ET_WEAPON:
		e->entPtr = &sg.weapons[sg.numWeapons];
		sg.numWeapons++;
		break;
	case ET_WALL:
		e->entPtr = &sg.wallEntity;
		break;
	};

	e->type = type;

	// link into the active list
	e->next = sg_activeEnts.next;
	e->prev = &sg_activeEnts;
	sg_activeEnts.next->prev = e;
	sg_activeEnts.next = e;
	
	return e;
}

void SG_ApplyKnockback(int dmgtype, sgentity_t *to, const sgentity_t *from)
{
	float direction;
	
	// clear the velocity
	to->vel[0] = 0;
	to->vel[1] = 0;
	to->vel[2] = 0;
	
	direction = atan2(from->vel[0], from->vel[1]) + M_PI;
	
	to->vel[0] = cos(direction);
	to->vel[1] = sin(direction);
	to->vel[2] = from->vel[2];
	
	to->vel[2] = cos(from->angle);
	
	//
	// some modifiers to how bad the knockback
	// is based on damage type
	//
	
	if (dmgtype == DMG_EXPLOSION) {
		float lenFactor = disBetweenOBJ(to->vel, from->vel);
		
		// target is right in the middle of the explosion
		if (BoundsIntersectPoint( from->bounds.mins, from->bounds.maxs, to->origin )) {
			lenFactor += 10.0f;
		}
		
		to->vel[0] *= lenFactor;
		to->vel[1] *= lenFactor;
		
		// add a bit more height
		to->vel[2] *= cos(4.0f);
	}
	
	to->state = &stateinfo[ ST_KNOCKBACK + to->stateOffset ];
}

void Ent_RunTic( void )
{
	sgentity_t *ent;

	for (ent = &sg_activeEnts; ent; ent = ent->next) {
		ent->ticker--;
		if (!ent->ticker) {
			Ent_SetState( ent, ent->state->next );
		}

		if (ent->state->action.acp1 == (actionf_p1)-1) {
			// remove it now
			SG_FreeEntity( ent );
		}
		else {
			if (ent->state->action.acp1) {
				ent->state->action.acp1( ent );
			}
		}
	}
}
