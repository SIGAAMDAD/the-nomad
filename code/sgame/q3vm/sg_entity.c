#include "sg_local.h"

sgentity_t sg_activeEnts;
sgentity_t *sg_freeEnts;

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
			SG_FreeEntity( self );
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
	// if we're touching anything with the side marked for collision, return true
	if ( G_CheckWallHit( &e->origin, d ) ) {
		return qtrue;
	}

	return qfalse;
}

void SG_BuildBounds( bbox_t *bounds, float width, float height, const vec3_t *origin )
{
	bounds->mins.x = origin->x - height;
	bounds->mins.y = origin->y - width;
	bounds->mins.z = origin->z;

	bounds->maxs.x = origin->x + height;
	bounds->maxs.y = origin->y + width;
	bounds->maxs.z = origin->z;
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

void SG_Spawn( uint32_t id, uint32_t type, const uvec3_t *origin )
{
	sgentity_t *ent;

	if ( sg.numEntities == MAXENTITIES ) {
		trap_Error( "SG_Spawn: MAXENTITIES hit" );
	}
	if ( type >= NUMENTITYTYPES ) {
		trap_Error( "SG_Spawn: unknown entity type" );
	}

	ent = &sg_entities[sg.numEntities];

	switch ( type ) {
	case ET_MOB:
		ent->classname = "mob";
		break;
	case ET_PLAYR: {
		if ( sg.playrReady ) {
			trap_Print( COLOR_YELLOW "WARNING: more than one player spawn in map, skipping.\n" );
			break;
		}
		ent->classname = "player";
		( (playr_t *)ent->entPtr )->ent = ent;
		SG_InitPlayer();
		break; }
	case ET_BOT: {
		ent->classname = "bot";
		break; }
	case ET_ITEM: {
		ent->classname = "item";
		ent->entPtr = SG_SpawnItem( id );
		( (item_t *)ent->entPtr )->ent = ent;
		break; }
	case ET_WEAPON: {
		ent->classname = "weapon";
		ent->entPtr = SG_SpawnWeapon( id );
		( (weapon_t *)ent->entPtr )->base->ent = ent;
		break; }
	case ET_WALL: {
		ent->classname = "wall";
		break; }
	};

	ent->type = type;
	VectorCopy( ent->origin, (*origin) );
	
	sg.numEntities++;
}

void SG_DamageEntity( sgentity_t *attacker, sgentity_t *victim )
{
	if ( victim->health < 0 ) {
		SG_KillEntity( attacker, victim );
	}
}

static void SG_GenerateObituary( sgentity_t *attacker, sgentity_t *victim )
{
	causeofdeath_t cod;
	const char *victim_name;
	const char *attacker_name;
	const char *death_string;

	if ( attacker->type == ET_PLAYR ) {
		if ( sg_moduleInfos.modules[sg.playr->curwpn->base.info->moduleIndex].dmage[sg.playr.curwpn->info->damagetype] )
	}

	SG_Printf( "%s" );
}

static void SG_GibEntity( sgentity_t *ent )
{

}

void SG_KillEntity( sgentity_t *attacker, sgentity_t *victim )
{
}

typedef struct {

} lerpFrame_t;

typedef struct {

} animation_t;

void SG_LoadAnimationFile( const char *filename )
{
}
