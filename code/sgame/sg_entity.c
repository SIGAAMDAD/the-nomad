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
		if ( BoundsIntersect( it->bounds.mins, it->bounds.maxs, ent->bounds.mins, ent->bounds.maxs ) && it != ent ) {
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
	bounds->mins[0] = origin[0] - height;
	bounds->mins[1] = origin[1] - width;
	bounds->mins[2] = origin[2];

	bounds->maxs[0] = origin[0] + height;
	bounds->maxs[1] = origin[1] + width;
	bounds->maxs[2] = origin[2];
}

void Ent_BuildBounds( sgentity_t *ent )
{
	ent->bounds.mins[0] = ent->origin[0] - ent->height;
	ent->bounds.mins[1] = ent->origin[1] - ent->width;
	ent->bounds.mins[2] = ent->origin[2];

	ent->bounds.maxs[0] = ent->origin[0] + ent->height;
	ent->bounds.maxs[1] = ent->origin[1] + ent->width;
	ent->bounds.maxs[2] = ent->origin[2];
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
