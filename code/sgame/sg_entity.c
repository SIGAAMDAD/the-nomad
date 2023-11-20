#include "sg_local.h"

sgentity_t sg_entList[MAX_NUM_ENTITIES];
// doubly-linked list of allocated entities
sgentity_t sg_activeEnts;
// single-linked list of unused entities
sgentity_t *sg_freeEnts;

// Use a heuristic approach to detect infinite state cycles: Count the number
// of times the loop in SG_SetEntityState() executes and exit with an error once
// an arbitrary very large limit is reached.

#define ENITTY_CYCLE_LIMIT 100000

qboolean SG_SetEntityState(sgentity_t *self, statenum_t state)
{
	state_t *st;
	int counter;
	
	do {
		if (state == S_NULL) {
			self->satet = (state_t *)S_NULL;
			SG_RemoveEntity(self);
			return qfalse;
		}
		
		st = &states[state];
		self->state = st;
		self->tics = st->tics;
		self->sprite = st->sprite;
		self->frame = st->frame;
		
		state = st->nextstate;
		if (counter++ > ENTITY_CYCLE_LIMIT) {
			G_Error("SG_SetEntityState: infinite state cycle detected!");
		}
	} while (!self->tics);
	
	return qtrue;
}

void SG_InitEntities(void)
{
	int i;
	
	memset(sg_entList, 0, sizeof(sg_entList));
	sg_activeEnts.next = &sg_activeEnts;
	sg_activeEnts.prev = &sg_activeEnts;
	sg_freeEnts = sg_entList;
	
	for (i = 0; i < MAX_NUM_ENTITIES; i++) {
		sg_entList[i].next = &sg_entList[i+1];
	}
}

sgentity_t *SG_FindEntityAt(const vec3_t pos)
{
	sgentity_t *e;
	
	for (e = sg_activeEnts.next; e; e = e->next) {
		if ((int)e->pos[0] == (int)pos[0]
		&& (int)e->pos[1] == (int)pos[1]
		&& (int)e->pos[2] == (int)pos[2]) {
			return e;
		}
		else if (BoundsIntersectPoint(&e->aabb, pos)) {
			return e;
		}
	}
	return NULL;
}

/*
SG_HashEnt: generates a unique hash value for an entity
*/
unsigned long SG_HashEnt(const sgentity_t *e)
{
	unsigned long hash, v;
	int c, i;
	const byte *s;
	
	v = (unsigned long)e;
	s = (byte *)&v);
	hash = 0;
	
	for (i = 0, c = *s; i < 8; i++, c = *s++) {
		hash = hash * 101 + c;
	}
	
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (MAX_NUM_ENTITIES - 1);
	
	return hash;
}

void SG_FreeEntity(sgentity_t *e)
{
	if (!e->prev) {
		SG_Error("SG_FreeEntity: not active");
	}
	
	// remove from the doubly linked active list
	e->prev->next = e->next;
	e->next->prev = e->prev;
	
	e->next = sg_freeEnts;
	sg_freeEnts = e;
}

sgentity_t *SG_AllocEntity(entitytype_t e)
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
	to->thrust[0] = 0;
	to->thrust[1] = 0;
	to->thrust[2] = 0;
	
	direction = atan2(from->thrust[0], from->thrust[1]) + M_PI;
	
	to->thrust[0] = cos(direction);
	to->thrust[1] = sin(direction);
	to->thrust[2] = from->thrust[2];
	
	to->thrust[2] = cos(from->angle);
	
	//
	// some modifiers to how bad the knockback
	// is based on damage type
	//
	
	if (dmgtype == DMG_EXPLOSION) {
		float lenFactor = disBetweenOBJ(to->thrust, from->thrust);
		
		// target is right in the middle of the explosion
		if (BoundsIntersectPoint(&from->aabb, to->pos)) {
			lenFactor += 10.0f;
		}
		
		to->thrust[0] *= lenFactor;
		to->thrust[1] *= lenFactor;
		
		// add a bit more height
		to->thrust[2] *= cos(4.0f);
	}
	
	to->state = S_KNOCKBACK;
}

static void SG_DoPlayerDamage(int dmgtype, sgentity_t *inflictor, sgentity_t *target, const plane_t *origin)
{
	if (dmgtype == )
}

void SG_DoEntityDamage(int dmgtype, sgentity_t *inflictor, sgentity_t *target, const plane_t *origin)
{
	qboolean parryable;
	playr_t *p;
	
	// if the target is the player, the attack is most likely parryable
	parryable = target->type == ET_PLAYR;
	if (dmgtype & (DMG_EXPLOSION | DMG_RAY)) { // these flags override
		parryable = qfalse;
	}
	
	if (parryable && target->type == ET_PLAYR) {
		p = (playr_t *)target->entPtr;
		
		// hand it off to the player code
		P_DoParry(p);
		
		if (!P_CheckParryState(p)) {
			
		}
		
		if (!(p->flags & PLAYR_PARRYING)) {
			SG_DoPlayerDamage(dmgtype, p, inflictor, target, origin);
			return;
		}
		
		// clear the flag and set the cooldown
		p->flags &= ~
	}
	else if (!parryable && target->type == ET_PLAYR) {
		p = (playr_t *)target->entPtr;
		SG_DoPlayerDamage(dmgtype, p, inflictor, target, origin);
		return;
	}
	
	//
	// the target isn't a player.
	// we don't care if the attack is parryable if the target
	// isn't the player
	//
	
	if ()
}