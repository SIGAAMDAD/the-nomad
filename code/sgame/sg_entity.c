#include "sg_local.h"

sgentity_t sg_entities[MAXENTITIES];

// Use a heuristic approach to detect infinite state cycles: Count the number
// of times the loop in Ent_SetState() executes and exit with an error once
// an arbitrary very large limit is reached.

#define ENTITY_CYCLE_LIMIT 100000

qboolean Ent_SetState( sgentity_t *self, statenum_t state )
{
	state_t *st;
	int counter;

	counter = 0;
	do {
		if ( state == S_NULL ) {
			self->state = &stateinfo[S_NULL];
			SG_FreeEntity(self);
			return qfalse;
		}

		st = &stateinfo[state];
		self->state = st;
		self->ticker = st->tics;
		self->frame = st->frames;
		self->sprite = st->sprite + self->facing;

		state = st->nextstate;
		if ( counter++ > ENTITY_CYCLE_LIMIT ) {
			G_Error("Ent_SetState: infinite state cycle detected!");
		}
	} while ( !self->ticker );

	return qtrue;
}

sgentity_t *Ent_CheckEntityCollision( const sgentity_t *ent )
{
	sgentity_t *it;
	int i;

	it = &sg_entities[0];
	for ( i = 0; i < sg.numEntities; i++, it++ ) {
		if ( BoundsIntersect( &it->bounds, &ent->bounds ) && it != ent ) {
			return it;
		}
	}

	return NULL;
}

/*
 * Ent_CheckWallCollision: returns true if there's a wall in the way
 */
qboolean Ent_CheckWallCollision( const sgentity_t *e )
{
	dirtype_t d;

	switch ( e->dir ) {
	case DIR_NORTH:
	case DIR_SOUTH:
	case DIR_EAST:
	case DIR_WEST:
		d = inversedirs[e->dir];
		break;
	case DIR_NORTH_WEST:
	case DIR_NORTH_EAST:
	case DIR_SOUTH_WEST:
	case DIR_SOUTH_EAST:
		return qtrue;
	};

	// check for a wall collision
	// if we're touching a wall with the side marked for collision, return true
	if ( trap_CheckWallHit( e->origin, d ) ) {
		return qtrue;
	}

	return qfalse;
}

void SG_BuildBounds( bbox_t *bounds, float width, float height, const vec3_t origin )
{
	bounds->mins.x = origin.x - height;
	bounds->mins.y = origin.y - width;
	bounds->mins.z = origin.z;

	bounds->maxs.x = origin.x + height;
	bounds->maxs.y = origin.y + width;
	bounds->maxs.z = origin.z;
}

void Ent_BuildBounds( sgentity_t *ent )
{
	ent->bounds.mins.x = ent->origin.x - ent->height;
	ent->bounds.mins.y = ent->origin.y - ent->width;
	ent->bounds.mins.z = ent->origin.z;

	ent->bounds.maxs.x = ent->origin.x + ent->height;
	ent->bounds.maxs.y = ent->origin.y + ent->width;
	ent->bounds.maxs.z = ent->origin.z;
}

void SG_FreeEntity( sgentity_t *ent )
{
	if ( !ent->health ) {
		trap_Print( COLOR_YELLOW "WARNING: SG_FreeEntity: freed a freed entity" );
		return;
	}

	sg.numEntities--;
}

sgentity_t *SG_AllocEntity( entitytype_t type )
{
	sgentity_t *ent;

	if ( sg.numEntities == MAXENTITIES ) {
		trap_Error( "SG_AllocEntity: MAXENTITIES hit" );
	}

	ent = &sg_entities[sg.numEntities];
	sg.numEntities++;

	memset( ent, 0, sizeof(*ent) );
	ent->type = type;
	ent->ticker = -1;
	ent->width = ent->height = 0;

	switch ( type ) {
	case ET_MOB:
		ent->classname = "mob";
		break;
	case ET_ITEM:
		ent->classname = "item";
		break;
	case ET_BOT:
		ent->classname = "bot";
		break;
	case ET_PLAYR:
		ent->classname = "player";
		break;
	case ET_WALL:
		ent->classname = "wall";
		break;
	case ET_WEAPON:
		ent->classname = "weapon";
		break;
	};

	Ent_BuildBounds( ent );

	return ent;
}
